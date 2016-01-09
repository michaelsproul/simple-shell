#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H

void ** new_array(unsigned * size);
int push(void *** array, void * value, unsigned * num_elements, unsigned * size);
void free_array(void ** array, unsigned num_elements);

#endif
