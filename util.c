#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* Based on code from http://stackoverflow.com/questions/482375/strdup-function */
char * strdup(const char * s) {
    unsigned length = strlen(s) + 1;
    char * p = malloc(length * sizeof(char));
    if (p != NULL) {
        strncpy(p, s, length);
    }
    return p;
}

/* Reset program_args by freeing all of its argument vectors.
 * The array itself is NOT freed.
*/
void reset_all_program_args(char *** program_args, unsigned * num_program_args) {
    unsigned i;
    unsigned j;
    for (i = 0; i < *num_program_args; i++) {
        /* Free every string in the argument array (stop on NULL end marker). */
        for (j = 0; program_args[i][j] != NULL; j++) {
            free(program_args[i][j]);
        }
        /* Free the array used to hold the arguments. */
        free(program_args[i]);
    }
    *num_program_args = 0;
}
