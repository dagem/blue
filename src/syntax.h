#ifndef __BLUE_SYNTAX_H__
#define __BLUE_SYNTAX_H__

#include "editor.h"

enum editorHighlight
{
	HL_NORMAL = 0,
	HL_NUMBER,
	HL_MATCH
};
void editorUpdateSyntax(erow *row);
void editorSelectSyntaxHighlight();
int editorSyntaxToColor(int hl);
int is_separator(int c);

#endif /*__BLUE_SYNTAX_H__*/