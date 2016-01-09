#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "util.h"
#include "dynamic_array.h"

/* Macros anyone? */

/* Read to the end of the current line.  */
#define finish_line()   while (yylex() != Newline)

/* Print an error message, free the temporary buffer and return. */
#define error(message)  print_error_and_free(message); \
                        finish_line(); \
                        return Error;

#define print_error_and_free(message) \
    fprintf(stderr, "parse error: %s\n", message); \
    fflush(NULL); \
    free_array((void **) *current_prog_args, *num_current_prog_args);

/* Return an error if the given argument is false. */
#define try(x, message) if (!(x)) { error(message) }

/* Attempt to push an element onto an array. Error if the push fails. */
#define try_push(array, value) try(push((void ***) array, \
                                        (void *) value, \
                                        num_ ## array, \
                                        array ## _size), \
                                        "unable to allocate memory.")

/* Mark current_program_args as complete and add it to the global list.
 * args : char *** (pointer to stack variable containing pointer to heap array).
*/
#define add_to_program_args(args)   try_push(args, NULL); \
                                    try_push(program_args, *args)

/* Declare a pointer to an unsigned stack variable in one line. */
#define declare_unsigned_ptr(name)  unsigned _ ## name = 0; \
                                    unsigned * name = &_ ## name


Result parse(
    char **** program_args,
    unsigned * num_program_args,
    unsigned * program_args_size,
    char ** file_in,
    char ** file_out) {

    /* Token variables. */
    int current_token;

    /* Parsing tracking. */
    int expecting_prog_name = 0;
    int file_in_allowed = 1;
    int file_out_allowed = 1;
    int pipe_allowed = 1;

    /* An array of arguments for the current program.
     * This array is added to the array of arrays when a new program is encountered.
    */
    declare_unsigned_ptr(num_current_prog_args);
    declare_unsigned_ptr(current_prog_args_size);
    char ** current_prog_args_r;
    char *** current_prog_args = &current_prog_args_r;

    /* Parse the first token (should be a program name). */
    current_token = yylex();

    /* Allow empty commands. */
    if (current_token == Newline) {
        return Empty;
    }
    /* End of input should always be caught here (CTRL-D or end of script). */
    else if (current_token == EndOfFile) {
        printf("\n");
        return Exit;
    }
    /* Handle the exit command here (should have 0 arguments). */
    else if (strncmp(yytext, "exit", 4) == 0 && yylex() == Newline) {
        return Exit;
    }
    /* Handle cd as well (one argument). */
    else if (strncmp(yytext, "cd", 2) == 0) {
        Result return_val = Empty;
        int first_token;
        int second_token;
        char * directory;

        first_token = yylex();

        if (first_token == Word) {
            directory = strdup(yytext);

            second_token = yylex();

            if (second_token == Newline) {
                int chdir_status = chdir(directory);
                if (chdir_status == -1) {
                    fprintf(stderr, "exec error: unable to cd.\n");
                    return_val = Error;
                }
            } else {
                fprintf(stderr, "parse error: cd only takes one argument.\n");
                finish_line();
                return_val = Error;
            }

            free(directory);
            return return_val;
        }

        if (first_token == Newline) {
            fprintf(stderr, "parse error: cd only takes one argument.\n");
            return_val = Error;
        } else {
            finish_line();
        }

        return return_val;
    }

    *file_in = NULL;
    *file_out = NULL;

    /* Allocate the array of arguments for the current program. */
    *current_prog_args = (char **) new_array(current_prog_args_size);

    /* Add the first program (parsed above) */
    try(current_token == Word, "no initial program to run.");
    try_push(current_prog_args, strdup(yytext));

    while (1) {
        current_token = yylex();

        switch (current_token) {
            case Word: {
                /* Handle the beginning of a new program. */
                if (expecting_prog_name) {
                    /* Add previous program args array to program args array of arrays. */
                    add_to_program_args(current_prog_args);

                    /* Allocate a new array of arguments for this program. */
                    *current_prog_args = (char **) new_array(current_prog_args_size);
                    *num_current_prog_args = 0;

                    expecting_prog_name = 0;
                    pipe_allowed = 1;
                    file_out_allowed = 1;
                }

                /* Push the argument onto the currrent list of arguments. */
                try_push(current_prog_args, strdup(yytext));
                break;
            }

            case FileOut: {
                if (file_out_allowed) {
                    try(yylex() == Word, "no valid filename provided to >");
                    *file_out = strdup(yytext);

                    file_out_allowed = 0;
                    pipe_allowed = 0;

                    break;
                }
                error("inappropriate redirection to a file.");
            }

            case FileIn: {
                if (file_in_allowed) {
                    try(yylex() == Word, "no valid filename provided to <");
                    *file_in = strdup(yytext);

                    file_in_allowed = 0;

                    break;
                }
                error("inappropriate redirection from a file.");
            }

            case Pipe: {
                if (pipe_allowed) {
                    expecting_prog_name = 1;
                    file_in_allowed = 0;
                    break;
                }
                error("inappropriate pipe.");
            }

            /* If a new line is reached, the line has been parsed successfully. */
            case Newline: {
                /* Check for pipes with no destination. */
                if (expecting_prog_name) {
                    print_error_and_free("no program following pipe.");
                    return Error;
                }

                /* Add the last set of program args to the array. */
                add_to_program_args(current_prog_args);

                return Ok;
            }

            case EndOfFile: {
                error("unreachable code executed!");
            }

            default: {
                printf("unhandled token: %s\n", yytext);
            }
        }
    }
}
