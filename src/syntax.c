#include "syntax.h"


int editorSyntaxToColor(int hl)
{
    switch(hl)
    {
        case HL_INCLUDE:
            return 30;
        case HL_MLCOMMENT:
        case HL_COMMENT:
            return 36;
        case HL_KEYWORD1:
            return 33;
        case HL_KEYWORD2:
            return 32;
        case HL_STRING:
            return 35;
        case HL_NUMBER:
            return 31;
        case HL_MATCH:
            return 34;
        default: 
            return 37;
    }
}
int is_separator(int c)
{
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}