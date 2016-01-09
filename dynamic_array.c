#include <stdlib.h>

#include "dynamic_array.h"

/* Create a new single-element array of pointer sized values. */
void ** new_array(unsigned * size) {
    void ** array = malloc(sizeof(void *));
    *size = 1;
    return array;
}

/*
 * Push a value onto an array of pointer sized values.
 *
 * `num_elements` is the number of elements in the array, while `size` is the underlying size.
 *
 * Return 1 on success and 0 on failure.
*/
int push(void *** array, void * value, unsigned * num_elements, unsigned * size) {
    /* If capacity has been reached, resize. */
    if (*num_elements == *size) {
        *size *= 2;
        *array = realloc(*array, (*size) * sizeof(void *));

        if (*array == NULL) {
            return 0;
        }
    }
    /* Now that there's space, push the element. */
    (*array)[*num_elements] = value;
    *num_elements += 1;

    return 1;
}

/* Free an array of pointer sized values where the values themselves can be freed using `free`. */
void free_array(void ** array, unsigned num_elements) {
    unsigned i;
    for (i = 0; i < num_elements; i++) {
        free(array[i]);
    }
    free(array);
}
