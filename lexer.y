%{

#include "parse.h"

%}

%option noyywrap

WORD [a-zA-Z0-9\/\.-]+
SPECIAL [()><|&;*]

%%

{WORD}  return Word;
"<"     return FileIn;
">"     return FileOut;
"|"     return Pipe;

\n      return Newline;

[ \t]+  {}
.       {}

<<EOF>> return EndOfFile;

%%

void flex_cleanup(void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
