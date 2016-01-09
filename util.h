#ifndef _UTIL_H
#define _UTIL_H

/* Generic result type for reporting parsing and execution errors. */
typedef enum {
    Ok,
    Empty,
    Error,
    Exit
} Result;

char * strdup(const char *s);

void reset_all_program_args(char *** program_args, unsigned * num_program_args);

#endif
