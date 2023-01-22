#include "editor.h"

static editorConfig E;

void die(const char*s)
{
	write(STDIN_FILENO, "\x1b[2J", 4);
	write(STDIN_FILENO, "\x1b[H", 3);
	
	perror(s);
	exit(1);
}
void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) ==1)
	{
		die ("tcsetattr");
	} //restores terminal upon exit
}

void enableRawMode()
{
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
	{
		die("tcgetattr");
	} //gets the original termios
	atexit(disableRawMode); //at exit call disablerawmode

	struct termios raw = E.orig_termios; // 
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //turns off CTRL+S and CTRL+Q's default(s) (Stop & Start process).
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //attributes applied to terminal (ISIG = Signal Handler), (ECHO = Print to screen), etc.
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	{
		die("tcsetattr");
	} //applies modifications to the terminal 

}
int editorReadKey()
{
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
	{
		if(nread == -1 && errno != EAGAIN)
		{
			die("read");
		}
	}
	if(c == '\x1b')
	{
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1)
		{
			return '\x1b';
		}
		if (read(STDIN_FILENO, &seq[1], 1) != 1)
		{
			return '\x1b';
		}
		if(seq[0] == '[')
		{
			if(seq[1] >= '0' && seq[1] <= '9')
			{
				if(read(STDIN_FILENO, &seq[2], 1) != 1)
				{
					return '\x1b';
				}
				if(seq[2] == '~')
				{
					switch (seq[1])
					{
						case '1': 
							return HOME_KEY;
						case '3':
							return DEL_KEY;
						case '4':
							return END_KEY;
						case '5': 
							return PAGE_UP;
						case '6':
							return PAGE_DOWN;
						case '7':
							return HOME_KEY;
						case '8':
							return END_KEY;
					}
				}
			}
			else
			{
				switch (seq[1])
				{
					case 'A': 
						return ARROW_UP;
					case 'B': 
						return ARROW_DOWN;
					case 'C':
						return ARROW_RIGHT;
					case 'D': 
						return ARROW_LEFT;
					case 'H':
						return HOME_KEY;
					case 'F':
						return END_KEY;
				}
			}
		}
		else if (seq[0] == 'O')
		{
			switch (seq[1])
			{
				case 'H':
					return HOME_KEY;
				case 'F':
					return END_KEY;
			}
		}
		return '\x1b';
	}
	else
	{
		return c;
	}
}
int getCursorPosition(int *rows, int *cols) 
{
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
  {
	return -1;
  }
  printf("\r\n");

  while(i < sizeof(buf) -1)
  {
	if (read(STDIN_FILENO, &buf[i], 1) != 1)
	{
		break;
	}
	if(buf[i] == 'R')
	{
		break;
	}
	i++;
  }
  buf[i] = '\0';
  if(buf[0] != '\x1b' || buf[1] != '[')
  {
	return -1;
  }
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
  {
	return -1;
  }
  return 0;
}

int getWindowSize(int *rows, int *cols) 
{
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) 
  {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) 
	{
		return -1;
	}
    return getCursorPosition(rows, cols);
  }
  else 
  {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
int editorRowCxToRx(erow *row, int cx)
{
	int rx = 0;
	int j;
	for(j = 0; j < cx; j++)
	{
		if(row ->chars[j] == '\t')
		{
			rx += (BLUE_TAB_STOP - 1) - (rx % BLUE_TAB_STOP);
		}
		rx++;
	}
	return rx;
}
void editorUpdateRow(erow *row)
{
	int tabs = 0;
	int j;
	for(j = 0; j < row->size; j++)
	{
		if((row->render = malloc(row->size + tabs*7 + 1) ))
		{
			tabs++;
		}
	}
	free(row ->render);
	row->render = malloc(row->size + tabs*(BLUE_TAB_STOP-1) + 1);
	int idx = 0;
	for(j = 0; j < row->size; j++)
	{
		if(row->chars[j] == '\t')
		{
			row->render[idx++] = ' ';
			while(idx % BLUE_TAB_STOP != 0)
			{
				row->render[idx++] = ' ';
			}
		}
		else
		{
			row ->render[idx++] = row->chars[j];
		}
	}
	row->render[idx] = '\0';
	row->rsize = idx;
}
void editorInsertRow(int at, char *s, size_t len)
{
	if(at < 0 || at > E.numrows)
	{
		return;
	}
	E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
	memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));	
	E.row[at].size = len;
	E.row[at].chars = malloc(len + 1);
	memcpy(E.row[at].chars, s, len);
	E.row[at].chars[len] = '\0';
	
	E.row[at].rsize = 0;
	E.row[at].render = NULL;
	editorUpdateRow(&E.row[at]);
	
	E.numrows++;
	E.dirty++;
}
void editorRowInsertChar(erow *row, int at, int c)
{
	if(at < 0 || at > row->size)
	{
		at = row->size;
	}
	row ->chars = realloc(row->chars, row->size + 2);
	memmove(&row->chars[at+1], &row->chars[at], row->size - at + 1);
	row->size++;
	row->chars[at] = c;
	
	editorUpdateRow(row);
	E.dirty++;
}
void editorRowDelChar(erow *row, int at)
{
	if(at < 0 || at >= row->size)
	{
		return;
	}
	memmove(&row->chars[at], &row->chars[at+1], row->size-at);
	row->size--;
	editorUpdateRow(row);
	E.dirty++;
}
void editorInsertChar(int c)
{
	if (E.cy == E.numrows)
	{
		editorInsertRow(E.numrows,"", 0);
	}
	editorRowInsertChar(&E.row[E.cy], E.cx, c);
	E.cx++;
}
void editorDelChar()
{
	if(E.cy == E.numrows)
	{
		return;
	}
	if(E.cx == 0 && E.cy == 0)
	{
		return;
	}
	erow *row = &E.row[E.cy];
	if(E.cx > 0)
	{
	editorRowDelChar(row, E.cx-1);
	E.cx--;
	}
	else
	{
		E.cx = E.row[E.cy - 1].size;
		editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
		editorDelRow(E.cy);
		E.cy--;
	}
}
void editorFreeRow(erow* row)
{
	free(row->render);
	free(row->chars);
}
void editorDelRow(int at)
{
	if(at < 0 || at >= E.numrows)
	{
		return;
	}
	editorFreeRow(&E.row[at]);
	memmove(&E.row[at], &E.row[at+1], sizeof(erow) * (E.numrows-at-1));
	E.numrows--;
	E.dirty++;
}
void editorRowAppendString(erow *row, char *s, size_t len)
{
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row ->chars[row->size], s, len);
	row->size +=len;
	row->chars[row->size] = '\0';
	editorUpdateRow(row);
	E.dirty++;
}
void editorInsertNewline()
{
	if(E.cx == 0)
	{
		editorInsertRow(E.cy, "", 0);
	}
	else
	{
		erow *row = &E.row[E.cy];
		editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
		row = &E.row[E.cy];
		row->size = E.cx;
		row->chars[row->size] = '\0';
		editorUpdateRow(row);
	}
	E.cy++;
	E.cx = 0;
}
void abAppend(struct abuf *ab, const char *s, int len)
{
	char *new = realloc(ab->b, ab->len + len);
	if(new == NULL)
	{
		return;
	}
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}
void abFree(struct abuf *ab)
{
	free(ab->b);
}
void editorMoveCursor(int key)
{
	erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
	switch(key)
	{
		case ARROW_LEFT:
			if(E.cx != 0){
				E.cx--;
			}
			else if(E.cy > 0)
			{
				E.cy--;
				E.cx = E.row[E.cy].size;
			}
			break;
		case ARROW_RIGHT:
			if(row && E.cx < row->size)
			{
				E.cx++;
			}
			else if (row && E.cx == row->size)
			{
				E.cy++;
				E.cx = 0;
			}
			break;
		case ARROW_UP:
			if(E.cy != 0)
				E.cy--;
			break;
		case ARROW_DOWN:
			if(E.cy < E.numrows)
				E.cy++;
			break;
	}
	row = (E.cy >= E.numrows) ? NULL: &E.row[E.cy];
	int rowlen = row ? row->size: 0;
	if(E.cx > rowlen)
	{
		E.cx = rowlen;
	}
}
char* editorPrompt(char *prompt)
{
	size_t bufsize = 128;
	char *buf = malloc(bufsize);

	size_t buflen = 0;
	buf[0] = '\0';

	while(1)
	{
		editorSetStatusMessage(prompt, buf);
		editorRefreshScreen();

		int c = editorReadKey();
		if (c == '\x1b')
		{
			editorSetStatusMessage("");
			free(buf);
			return NULL;
		}
		else if(c == '\r')
		{
			editorSetStatusMessage("");
			return buf;
		}
		else if (!iscntrl(c) && c < 128)
		{
			if(buflen == bufsize - 1)
			{
				bufsize *= 2;
				buf = realloc(buf, bufsize);
			}
			buf[buflen++] = c;
			buf[buflen] = '\0';
		}
		
	}
}
char* editorRowsToString(int *buffer_len)
{
	int total_len = 0;
	int j = 0;
	for(j = 0; j < E.numrows; j++)
	{
		total_len += E.row[j].size + 1;
	}
	*buffer_len = total_len;

	char *buffer = malloc(total_len);
	char *pointer = buffer;
	for(j = 0; j < E.numrows; j++)
	{
		memcpy(pointer, E.row[j].chars, E.row[j].size);
		pointer += E.row[j].size;
		*pointer = '\n';
		pointer++;
	}
	return buffer;
}
void editorSave()
{
	if(E.filename == NULL)
	{
		E.filename = editorPrompt("Save as: %s (ESC to cancel)");
		if(E.filename == NULL)
		{
			editorSetStatusMessage("Save aborted");
			return;
		}
	}
	
	int length;
	char *buf = editorRowsToString(&length);

	int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1)
	{
		if(ftruncate(fd, length) != -1)
		{
			if(write(fd, buf, length) == length)
			{
				close(fd);
				free(buf);
				E.dirty = 0;
				editorSetStatusMessage("File: '%s' was sucessfuly saved, %d bytes written to disk", E.filename, length);
				return;
			}
		}
		close(fd);
	}
	free(buf);
	editorSetStatusMessage("Save failed! I/O error: %s", strerror(errno));
}
void editorProcessKeypress()
{
	int c = editorReadKey();
	static int quit_times = BLUE_QUIT_TIMES;
	char* str = malloc(sizeof(char) * 255);

	switch(c)
	{
		case '\r':
			editorInsertNewline();
			break;
		case CTRL_KEY('q'):
			
			if((E.dirty && quit_times) > 0)
			{
				
				if(E.filename == NULL)
				{
					str  = "unnamed file\0";
				}
				else
				{
					strcpy(str, E.filename);
				}
				editorSetStatusMessage("WARNING!! %s has unsaved changes. Press Ctrl-Q %d more times to quit", str, quit_times);
				quit_times--;
				return;
				
			}
			write(STDOUT_FILENO, "\x1b[2J", 4);
     		write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
		break;
		case HOME_KEY:
			E.cx = 0;
			break;
		case END_KEY:
			if(E.cy < E.numrows)
			{
				E.cx = E.row[E.cy].size;
			}
			break;
		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL_KEY:
			if(c == DEL_KEY)
			{
				editorMoveCursor(ARROW_RIGHT);
			}
			editorDelChar();
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			{
				if(c == PAGE_UP)
				{
					E.cy = E.rowoff;
				}
				else if(c == PAGE_DOWN)
				{
					E.cy = E.rowoff + E.screenrows - 1;
				}
				if(E.cy > E.numrows)
				{
					E.cy = E.numrows;
				}
				int times = E.screenrows;
				while (times--)
				{
				editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
				}
			}
			break;
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editorMoveCursor(c);
			break;
		case CTRL_KEY('l'):
		case '\x1b':
			break;
		case CTRL_KEY('s'):
			editorSave();
			break;
		default: 
			editorInsertChar(c);
			break;
	}
	quit_times = BLUE_QUIT_TIMES;
	free(str);
}
void editorScroll()
{
	E.rx = E.cx;
	if (E.cy < E.numrows)
	{
		E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
	}
	if(E.cy < E.rowoff)
	{
		E.rowoff = E.cy;
	}
	if(E.cy >= E.rowoff + E.screenrows)
	{
		E.rowoff = E.cy - E.screenrows + 1;
	}
	if(E.rx < E.coloff)
	{
		E.coloff = E.cx;
	}
	if(E.rx >= E.coloff + E.screencols)
	{
		E.coloff = E.rx - E.screencols + 1;
	}
}
void editorDrawRows(struct abuf *ab)
{
	int i;
	for(i = 0; i < E.screenrows; i++)
	{
		int filerow = i + E.rowoff;
		if(filerow >= E.numrows)
		{
			if(E.numrows == 0 && i == E.screenrows/3)
			{
				char welcome [80];
				int welcomelen = snprintf(welcome, sizeof(welcome),
				"BLUE editor -- version %s", BLUE_VERSION);
				if (welcomelen > E.screencols)
				{
					welcomelen = E.screencols;
				}
				int padding = (E.screencols - welcomelen) / 2;
				if(padding)
				{
					abAppend(ab, ">", 1);
					padding--;
				}
				while(padding--)
				{
					abAppend(ab, " ", 1);
				}
				abAppend(ab, welcome, welcomelen);
			}
			else
			{
				abAppend(ab, ">", 1);
			}
		}
		else
		{
			int len = E.row[filerow].rsize - E.coloff;
			if(len < 0)
			{
				len = 0;
			}
			if (len > E.screencols)
			{
				len = E.screencols;
			}
			abAppend(ab, &E.row[filerow].render[E.coloff], len);
		}
		abAppend(ab, "\x1b[K", 3);
		abAppend(ab,"\r\n", 2);
	}
}
void editorDrawStatusBar(struct abuf *ab)
{
	abAppend(ab, "\x1b[7m", 4);
	char status[80], rstatus[80];
	int len = snprintf(status, sizeof(status), "%.20s - %d lines",
					   E.filename ? E.filename : "[No Name]", E.numrows,
					   E.dirty ? "(modified)" : "");
	int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
						E.cy + 1, E.numrows);
	if(len > E.screencols)
	{
		len = E.screencols;
	}
	abAppend(ab,status,len);
	while(len < E.screencols)
	{
		if(E.screencols - len == rlen)
		{
			abAppend(ab,rstatus,rlen);
			break;
		}
		else
		{
			abAppend(ab, " ", 1);
			len++;
		}
	}
	abAppend(ab, "\x1b[m", 3);
	abAppend(ab, "\r\n", 2);
}
void editorDrawMessageBar(struct abuf *ab)
{
	abAppend(ab, "\x1b[K", 3);
	int msglen = strlen(E.statusmsg);
	if(msglen > E.screencols)
	{
		msglen = E.screencols;
	}
	if(msglen && time(NULL) - E.statusmsg_time < 5)
	{
		abAppend(ab, E.statusmsg, msglen);
	}
}
void editorRefreshScreen()
{
	editorScroll();
	struct abuf ab = ABUF_INIT;

	abAppend(&ab, "\x1b[?25l", 6);

	abAppend(&ab, "\x1b[H", 3);

	editorDrawRows(&ab);
	editorDrawStatusBar(&ab);
	editorDrawMessageBar(&ab);

	char buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, 
											  (E.rx - E.coloff) + 1);
	abAppend(&ab, buf, strlen(buf));

	abAppend(&ab, "\x1b[?25h",6);

	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}
void editorOpen(char *filename)
{
	free(E.filename);
	E.filename = strdup(filename);

	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		die("fopen");
	}
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	while((linelen = getline(&line, &linecap, fp)) != -1)
	{
		while(linelen > 0 && (line[linelen - 1] == '\n' || 
							  line[linelen - 1] == '\r'))
		{
			linelen--;
		}
		editorInsertRow(E.numrows,line, linelen);
	}
	free(line);
	fclose(fp);
	E.dirty = 0;
}
void editorSetStatusMessage(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}
void initEditor()
{
	E.cx = 0;
	E.cy = 0;
	E.rx = 0;
	E.rowoff = 0;
	E.coloff = 0;
	E.numrows = 0;
	E.dirty = 0;
	E.row = NULL;
	E.filename = NULL;
	E.statusmsg[0] = '\0';
	E.statusmsg_time = 0;
	if(getWindowSize(&E.screenrows, &E.screencols) == -1)
	{
		die("getWindowSize");
	}
	E.screenrows-=2;
}