/*
    (C) Aleksander Zdunek <redacted>@cs.umu.se
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <assert.h>
#include "input.h"

#define PIPE_FD_IDX_READ 0
#define PIPE_FD_IDX_WRITE 1

bool exec_command(char** argv, int pipe_fd_in, int pipe_fd_out);
int reap_children(void);
bool create_pipes(int* fd, size_t count);
void close_fds(int* fd, size_t count);
bool run_pipeline(char*** commands, size_t nrof_commands, int* pipe_fds);

bool g_main_process = true;

int main(int argc, char* argv[])
{
    //TODO: break out argument handling?
    FILE* infile = stdin;
    if( 2 == argc )
    {
        infile = fopen(argv[1], "r");
        if( !infile )
        {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
    }
    else if( argc > 2 )
    {
        fprintf(stderr, "usage: %s [FILE]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    size_t nrof_commands = 0;
    char*** commands = get_command_lines(infile, &nrof_commands); //I'm a three star programmer now!
    if(infile != stdin) fclose(infile);
    if(!commands)
    {
        fprintf(stderr, "Error parsing command lines\n");
        exit(EXIT_FAILURE);
    } else if(!*commands)
    {
        fprintf(stderr, "Nothing to pipe\n");
        free_command_lines(commands);
        exit(EXIT_FAILURE);
    }

    int pipe_fds[2 * nrof_commands];
    if(!create_pipes(pipe_fds, sizeof(pipe_fds)/sizeof(pipe_fds[0])))
    {
        free_command_lines(commands);
        exit(EXIT_FAILURE);
    }

    int exit_status = EXIT_SUCCESS;
    if(!run_pipeline(commands, nrof_commands, pipe_fds))
    {
        exit_status = EXIT_FAILURE;
    }

    free_command_lines(commands);
    close_fds(pipe_fds, sizeof(pipe_fds)/sizeof(pipe_fds[0]));
    if(g_main_process) //Only parent process waits for children.
    {
        int child_exit_status = reap_children();
        //Don't overwrite exit status if it's already been set to EXIT_FAILURE elsewhere
        if( EXIT_SUCCESS == exit_status )
        {
            exit_status = child_exit_status;
        }
    }
    return exit_status;
}

/*
    Fork and exec pipeline command.

    @param argv Pointer to array of string pointers. The first string pointer
        argv[0] is the command to execute. The rest are arguments to the
        command. The last string pointer shall be NULL.
    @param pipe_fd_in File descriptor for read end of the input pipe to the
        command. May be STDIN_FILENO if command is the first command in the
        pipeline.
    @param pipe_fd_out File descriptor for write end of output pipe for the
        command. May be STDOUT_FILENO if command is the first command in
        the pipeline.

    @return
        true in parent process on successful fork
        false on error
        Does not return in child process on succesfull exec
        Also sets the global g_main_process flag to false in child process after
        successful fork.
*/
bool exec_command(char** argv, int pipe_fd_in, int pipe_fd_out)
{
    const pid_t pid = fork();
    if( pid == -1 ) //Error
    {
        perror("fork()");
        return false;
    }
    else if( pid == 0 ) //Child
    {
        g_main_process = false;

        if( dup2(pipe_fd_in, STDIN_FILENO) == -1 )
        {
            perror("Error duplicating pipe input file descriptor");
            return false;
        }

        if( dup2(pipe_fd_out, STDOUT_FILENO) == -1 )
        {
            perror("Error duplicating pipe output file descriptor");
            close(STDIN_FILENO);
            return false;
        }

        execvp(argv[0], argv);
        perror(argv[0]);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        return false;
    }
    else //Parent
    {
        return true;
    }
}

/*
    Wait for child processes

    @return
        Returns 0 if all children exited with code 0
        Returns the least significant 8 bits of a child's exit code if a child
        exited with non-zero exit code.
        Returns the signal number + 128 if a child was terminated by a signal.
        If multiple child processes terminated with non-zero exit code or by
        a signal the return value represents the first process that happend to
        get reaper. Which process that is is indeterminate.
*/
int reap_children(void)
{
    int retval = 0;
    while(1)
    {
        int wstatus;
        pid_t pid = wait(&wstatus);
        if( -1 == pid )
        {
            if( ECHILD == errno ) //No more children
            {
                return retval;
            } else
            {
                perror("wait()");
            }
        } else
        {
            if(WIFEXITED(wstatus)) //Child terminated normally
            {
                if(WEXITSTATUS(wstatus) && !retval)
                {
                    retval = WEXITSTATUS(wstatus);
                }
            }
            else if(WIFSIGNALED(wstatus)) //Child was terminated by a signal
            {
                if(!retval)
                {
                    retval = 128 + WTERMSIG(wstatus);
                }
                if(WCOREDUMP(wstatus))
                {
                    fprintf(stderr, "Child pid %d dumped core\n", pid);
                }
            }
            else //I don't expect these to happen. Including for completeness.
            {
                if(WIFSTOPPED(wstatus)) //Child was stopped by a signal
                {
                    fprintf(stderr, "Child pid %d was stopped by signal %d\n", pid, WSTOPSIG(wstatus));
                }
                if(WIFCONTINUED(wstatus))
                {
                    fprintf(stderr, "Child pid %d was continued by SIGCONT\n", pid);

                }
            }
        }
    }
}

/*
    TODO: Document
*/
bool create_pipes(int* fd, size_t count)
{
    assert(count % 2 == 0); //Should be an even number
    memset(fd, 0, count * sizeof(fd[0]));
    fd[0] = STDIN_FILENO;
    fd[count - 1] = STDOUT_FILENO;
    for(size_t i = 1; i < count - 1; i += 2)
    {
        int pipefd[2];
        if(pipe(pipefd))
        {
            perror("Error creating pipe");
            close_fds(fd, count);
            return false;
        }
        fd[i] = pipefd[PIPE_FD_IDX_WRITE];
        fd[i+1] = pipefd[PIPE_FD_IDX_READ];
    }
    return true;
}

/*
    TODO: Document
*/
void close_fds(int* fd, size_t count)
{
    for(size_t i = 0; i < count; ++i)
    {
        if(fd[i] > STDERR_FILENO)
        {
            close(fd[i]);
            fd[i] = STDIN_FILENO;
        }
    }
}

/*
    TODO: Document
*/
bool run_pipeline(char*** commands, size_t nrof_commands, int* pipe_fds)
{
    for(size_t i = 0; i < nrof_commands; ++i)
    {
        int* fds = &pipe_fds[2*i];
        bool ok = exec_command(commands[i], fds[PIPE_FD_IDX_READ], fds[PIPE_FD_IDX_WRITE]);
        close_fds(fds, 2);
        if(!ok) return false;
    }
    return true;
}
