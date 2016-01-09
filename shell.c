#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynamic_array.h"
#include "parse.h"
#include "exec.h"
#include "util.h"

/* ANSI Terminal colour codes. */
#define BLUE "\x1B[1;34m"
#define WHITE "\x1B[1;37m"
#define RED "\x1B[1;31m"
#define NORMAL "\x1B[0m"

void flex_cleanup(void);

void print_prompt(void) {
    printf(BLUE "je " WHITE "suis " RED "charlie " WHITE "$ " NORMAL);
    fflush(NULL);
    return;
}

#define free_iteration_data()   reset_all_program_args(program_args, &num_program_args); \
                                free(file_in); \
                                free(file_out)

int main(void) {
    unsigned program_args_size = 0;
    unsigned num_program_args = 0;
    char *** program_args = (char ***) new_array(&program_args_size);
    char * file_in = NULL;
    char * file_out = NULL;

    Result parse_result;
    Result exec_result;
    while (1) {
        print_prompt();

        /* Parse the current line. */
        parse_result = parse(&program_args, &num_program_args, &program_args_size,
                             &file_in, &file_out);

        /* Do nothing on empty commands (loop around). */
        if (parse_result == Empty) {
            continue;
        }
        /* Break from the loop on Ctrl-D or 'exit'. */
        else if (parse_result == Exit) {
            break;
        }
        /* In the case of an error, free the successfuly parsed arguments and continue. */
        else if (parse_result == Error) {
            free_iteration_data();
            continue;
        }

        #ifdef DEBUG
        printf("Parsed Program Arguments:\n");
        unsigned i;
        for (i = 0; i < num_program_args; i++) {
            printf("> ");
            for (unsigned j = 0; program_args[i][j] != NULL; j++) {
                printf("%s ", program_args[i][j]);
            }
            printf("\n");
        }
        fflush(stdout);
        #endif

        /* Execute the command. */
        exec_result = execute(program_args, file_in, file_out, num_program_args);

        /* Free the program argument vectors and their contents, but retain the capacity of
         * program_args.
         */
        free_iteration_data();

        /* If the exec failed fatally, allow the child to exit. */
        if (exec_result == Exit) {
            break;
        }
    }

    free(program_args);
    flex_cleanup();

    return 0;
}
