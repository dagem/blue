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
int editorSyntaxToColor(int hl);
#endif /*__BLUE_SYNTAX_H__*/