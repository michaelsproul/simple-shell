#ifndef _PARSE_H
#define _PARSE_H

#include "util.h"

int yylex(void);
extern char * yytext;

typedef enum {
    Word = 150,
    FileOut = 151,
    FileIn = 152,
    Pipe = 153,
    Newline = 154,
    EndOfFile = 155
} Token;

Result parse(
    char **** program_args,
    unsigned * num_program_args,
    unsigned * program_args_size,
    char ** file_in,
    char ** file_out
);

#endif
