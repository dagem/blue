#ifndef __BLUE_SYNTAX_H__
#define __BLUE_SYNTAX_H__

#include "editor.h"

enum editorHighlight
{
	HL_NORMAL = 0,
	HL_COMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};
struct editorSyntax
{
	char *filetype;
	char **filematch;
	char **keywords;
	char *singleline_comment_start;
	int flags;
};
void editorUpdateSyntax(erow *row);
void editorSelectSyntaxHighlight();
int editorSyntaxToColor(int hl);
int is_separator(int c);

#endif /*__BLUE_SYNTAX_H__*/