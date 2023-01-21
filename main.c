#include "blue-support.h"
int main(int argc, char *argv[])
{
	enableRawMode();
	initEditor();
	if(argc >= 2)
	{
		editorOpen(argv[1]);
	}
	editorSetStatusMessage("HELP: CTRL-S to save, CTRL-Q to quit");
	while(1)
	{
		editorRefreshScreen();
		editorProcessKeypress();
	}
	return 0;

}
