#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "exec.h"

const static int FD_STD = -5;
const static int FD_UNDEFINED = -6;

void replace_std_fd(int old_fd, int new_fd, char * name);
int open_file(int * fd, char * filename, char * operation, int flags);

#define return_status(status)   free(child_pids); \
                                return status

/* Execute the given command.
 * Return Error if a non-fatal error occurs and execution can continue.
 * Return Exit if a fatal error occurs that needs to result in process termination.
 *
*/
Result execute( char *** program_args,
                char * file_in,
                char * file_out,
                unsigned num_programs) {
    /* Each process requires an input and output file descriptor.
     * -5 represents normal stdin/stdout.
     * -6 represents undefined.
    */
    int input_fd = FD_UNDEFINED;
    int output_fd = FD_STD;
    int fd_to_close = FD_UNDEFINED;

    /* Child process PIDs are tracked for reporting purposes. */
    pid_t * child_pids = (pid_t *) malloc(num_programs * sizeof(pid_t));

    int current_pipe[2];

    unsigned i = 0;
    Result exec_status;
    Result return_val = Ok;
    int child_exit_status;
    int child_exit_code;

    for (i = 0; i < num_programs; i++) {
        /* First program. */
        if (i == 0) {
            input_fd = FD_STD;

            /* If the input file is non-null, open it for reading instead of stdin. */
            if (open_file(&input_fd, file_in, "reading", O_RDONLY) == -1) {
                return_status(Error);
            }
        }

        /* Last program. */
        if (i == num_programs - 1) {
            output_fd = FD_STD;

            /* If there's an output file, open it for writing. */
            if (open_file(&output_fd, file_out, "writing", O_WRONLY | O_CREAT) == -1) {
                return_status(Error);
            }
        }
        /* Not the last program.
         * At this point in every iteration, input_fd is set to something meaningful,
         * so we just need to set output_fd by opening a new pipe.
        */
        else {
            /* Pipe and check for failure. */
            if (pipe(current_pipe) == -1) {
                fprintf(stderr, "exec error: unable to pipe.\n");
                return_status(Error);
            }

            /* Output to the write end of the pipe. */
            output_fd = current_pipe[1];

            /* Close the read end of the output pipe in the child. */
            fd_to_close = current_pipe[0];
        }

        /* With file descriptors set up, execute the program. */
        exec_status = execute_single(program_args[i], input_fd, output_fd,
                                     fd_to_close, &child_pids[i]);

        /* Child only: propogate exec failure to the top so that memory can be freed. */
        if (exec_status == Exit) {
            return_status(Exit);
        }
        /* Parent only: continue to next command on fork failure. */
        else if (exec_status == Error) {
            return_status(Error);
        }

        /* The input for the next program is the output from the previous. */
        input_fd = current_pipe[0];
    }

    /* Wait for children to exit and report errors.
     * Children are collected in-order to facilitate error reporting,
     * however this does mean that children that die quickly may have to wait to be cleaned up.
    */
    for (i = 0; i < num_programs; i++) {
        waitpid(child_pids[i], &child_exit_status, 0);

        /* Child exited normally. */
        if (WIFEXITED(child_exit_status)) {
            /* Child exit code is non-zero. */
            child_exit_code = WEXITSTATUS(child_exit_status);
            if (child_exit_code != 0) {
                fprintf(stderr, "%s [%d] exited with code %d.\n",
                        program_args[i][0], i, child_exit_code
                );
                return_val = Error;
            }
        }
        /* Child exited abnormally. */
        else {
            fprintf(stderr, "%s [%d] exited abnormally.\n", program_args[i][0], i);
            return_val = Error;
        }
    }

    return_status(return_val);
}

/* Execute a program in a new child process, with the given input and output file descriptors.
 * The file descriptor fd_to_close is closed from the child, and the child's PID is stored in
 * the memory pointed to by child_pid.
*/
Result execute_single(  char ** args,
                        int input_fd,
                        int output_fd,
                        int fd_to_close,
                        pid_t * child_pid) {
    /* Create a child by forking. */
    int pid = fork();

    if (pid == -1) {
        fprintf(stderr, "exec error: unable to fork.\n");
        return Error;
    }

    /* Child */
    if (pid == 0) {
        /* Replace stdin if appropriate. */
        replace_std_fd(input_fd, 0, "stdin");

        /* Replace stdout if appropriate. */
        replace_std_fd(output_fd, 1, "stdout");

        /* Close the unwanted file descriptor. */
        if (fd_to_close != FD_UNDEFINED) {
            close(fd_to_close);
        }

        /* All being well, exec! */
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "exec error: call to execvp failed.\n");
            /* We now have two shells, so this one has to die. */
            return Exit;
        }

        /* Unreachable. */
        exit(EXIT_SUCCESS);
    }

    /* Parent */
    /* Close the file descriptors used by the child. */
    if (input_fd != FD_STD) {
        close(input_fd);
    }
    if (output_fd != FD_STD) {
        close(output_fd);
    }

    *child_pid = pid;

    return Ok;
}

/* Replace a named file descriptor, printing an error and exiting if an error occurs. */
void replace_std_fd(int new_fd, int old_fd, char * name) {
    if (new_fd != FD_STD) {
        if (close(old_fd) == -1) {
            fprintf(stderr, "exec error: unable to close %s in child.\n", name);
            exit(EXIT_FAILURE);
        }
        if (dup2(new_fd, old_fd) == -1) {
            fprintf(stderr, "exec error: unable to duplicate to %s in child.\n", name);
            exit(EXIT_FAILURE);
        }
    }
}

/* Open a file with the given flags, printing an error and returning -1 if something goes wrong.
 * The operation is a description (reading or writing) for error reporting.
*/
int open_file(int * fd, char * filename, char * operation, int flags) {
    /* If there's a file, open it for reading. */
    if (filename != NULL) {
        *fd = open(filename, flags, 0644);

        /* Check for open error. */
        if (*fd == -1) {
            fprintf(stderr, "exec error: unable to open '%s' for %s.\n", filename, operation);
            return -1;
        }
    }
    return 0;
}
