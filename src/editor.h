#ifndef __BLUE_EDITOR_H__
#define __BLUE_EDITOR_H__
#include "header.h"
#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)
#define HLDB_ENTRIES (sizeof(HLDB)/sizeof(HLDB[0]))

enum editorKey 
{
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN,
	BACKSPACE = 127,
	ENTER
};
typedef struct erow
{
	int idx;
	int size;
	int rsize;
	char *chars;
	char *render;
	unsigned char *hl;
	int hl_open_comment;
} erow;

typedef struct editorConfig
{
	int cx, cy;
	int rx;
	int rowoff;
	int coloff;
	int screenrows; 
	int screencols;
	int numrows;
	erow *row;
	int dirty;
	char *filename;
	char statusmsg[80];
	struct editorSyntax *syntax;
	time_t statusmsg_time;

	struct termios orig_termios;
}editorConfig;
typedef struct abuf
{
	char *b;
	int len;
}abuf;


void die(const char*s);
void disableRawMode();
void enableRawMode();
int getWindowSize(int *rows, int *cols);
int editorRowCxToRx(erow *row, int cx);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorInsertNewline();
void editorRowInsertChar(erow *row, int at, int c);
void editorRowDelChar(erow *row, int at);
void editorInsertChar(int c);
void editorDelChar();
void editorFreeRow(erow* row);
void editorDelRow(int at);
void editorRowAppendString(erow *row, char *s, size_t len);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
void editorMoveCursor(int key);
char *editorRowsToString(int *buffer_len);
void editorSave();
void editorFind();
int editorRowRxToCx(erow *row, int rx);
void editorProcessKeypress();
void editorScroll();
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen();
void editorOpen(char *filename);
void editorSetStatusMessage(const char *fmt, ...);
char *editorPrompt(char *prompt, void(*callback)(char*, int));
void editorFindCallback(char *query, int key);
void editorFind();
void initEditor();
#endif /*__BLUE_EDITOR_H__*/