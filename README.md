# Simple Shell

A simple shell written in C.

## Features

* Unlimited pipes.
* Unlimited arguments.
* Sensible file redirection.
* Changing directories.

## Usage

```
$ make
$ ./shell
```

![Shell Screenshot](http://i.imgur.com/fN6Medw.png)

## High-Level Description

There is a central loop in `shell.c` that repeatedly fetches commands from standard input and executes them. A flex tokeniser provides a token stream to the `parse` function, which ensures the input is well formed and stores all relevant information in the `program_args` variable. If parsing is successful, the main function passes the details of the command to be executed to the execute function, which does all piping and forking.

## Desirable Properties

* Parsing is separated from execution. The execution stage can assume well-formed input, and treat program data structures as read-only.
* No memory leaks. Failure is bubbled up and carefully handled.
* Important forking and piping is contained in a very small section of code in `exec.c`.
* Failure to execute a command just results in the main execution loop continuing to the next bit of input. The shell is not programmed to exit unless instructed to by the user.

## Parsing Details

I decided to allow input of the following form:

1. One or more programs, each with zero or more arguments.
2. File redirection only where sensible. It doesn't make sense to read in a file on anything but the first command, nor does it make sense to redirect output to a file on anything but the last command.
3. No program arguments following a file redirection operation.

To achieve this I had to implement dynamic arrays with `void *` hackery.

The main parsing function takes a `char **** program_args`, which can be thought of as a pointer to a dynamic array of dynamic string arrays. There's an extra level of redirection so that it can be re-assigned if adding elements to the array causes a resize.

## Execution Details

To support unlimited piping my shell iterates through each program to be run and creates a pipe for its output. At each iteration a file descriptor pointing to the read-end of the previous program's pipe is held by the main process. This file descriptor is used to replace the standard input of newly spawned programs (using the standard `fork` and `exec` method). The only special cases are the first and last program, which need not be distinct, which interact with `stdin` (or an input file) and `stdout` (or an output file). This nice simplicity arises from the limitation on file redirection enforced by the parser. The tradeoff seems fair given that file redirection in the middle of a pipeline is fairly uncommon and can be done with `tee` anyway.

Once all of the children have been spawned, the main process waits for each of them in order. Initially I was just using `wait`, but switched to `waitpid` so that I could report the names of failed processes.

## Builtins

There are two shell builtins, `cd` and `exit`. Both must be run on their own without piping or file redirection. This decision was made to keep thing as simple as possible. The calling format for `cd` is:

```
cd <directory>
```

The directory argument *must* be provided, calling `cd` without an argument will not go to `~` as the shell has no concept of home directories.
