#ifndef _EXEC_H
#define _EXEC_H

#include <sys/types.h>
#include "util.h"

Result execute( char *** program_args,
                char * file_in,
                char * file_out,
                unsigned num_programs);

Result execute_single(char ** args, int input_fd, int output_fd,
                      int fd_to_close, pid_t * child_pid);

#endif
