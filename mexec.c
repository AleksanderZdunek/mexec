#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

#define PIPE_FD_IDX_READ 0
#define PIPE_FD_IDX_WRITE 1

char** parse_line(const char* buffer);
void exec_command(char** argv, int pipe_fd_in, int pipe_fd_out);
void reap_children(void);

int main(int argc, char* argv[])
{
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

    //Read first command line
    char linebuf[1025];
    if( !fgets(linebuf, sizeof(linebuf), infile) )
    {
        if( feof(infile) )
        {
            fprintf(stderr, "Nothing to pipe\n");
        }
        if( ferror(infile) )
        {
            perror("Error reading first command line");
        }
        fclose(infile);
        exit(EXIT_FAILURE);
    }
    char** command_0 = parse_line(linebuf);
    if( !command_0 )
    {
        fclose(infile);
        exit(EXIT_FAILURE);
    }
    int pipe_fd_in = STDIN_FILENO;

    //Read next command line, create pipe, exec previous command
    while( fgets(linebuf, sizeof(linebuf), infile) )
    {
        char** command_next = parse_line(linebuf);
        if( !command_next )
        {
            //TODO: cleanup
            exit(EXIT_FAILURE);
        }

        //Create pipe
        int pipefd[2];
        if( pipe(pipefd) )
        {
            perror("Error creating pipe");
            //TODO: cleanup
            exit(EXIT_FAILURE);
        }

        //Exec previous command
        exec_command(command_0, pipe_fd_in, pipefd[PIPE_FD_IDX_WRITE]);

        //Shuffle up
        free(command_0);
        command_0 = command_next;
        if(STDIN_FILENO != pipe_fd_in) close(pipe_fd_in);
        pipe_fd_in = pipefd[PIPE_FD_IDX_READ];
        close(pipefd[PIPE_FD_IDX_WRITE]);
    }
    //TODO: error checking of fgets()

    //exec last command
    exec_command(command_0, pipe_fd_in, STDOUT_FILENO);

    //clean up
    fclose(infile);
    free(command_0);
    close(pipe_fd_in);
    reap_children();

    return EXIT_SUCCESS;
}

/*
    Parse a string into whitespace-separated tokens.

    @param buffer Pointer to null-terminated string to parse.

    @return Pointer to array of token pointers. This pointer should be freed.
        Last pointer in array is NULL.
*/
char** parse_line(const char* buffer)
{
    const char* const whitespace = " \f\n\r\t\v";
    buffer += strspn(buffer, whitespace); //Ignore leading whitespace

    //Count number of tokens to make it easier to tell apriori how much
    //memory we need to allocate for token pointers.
    char* tokbuf = strdup(buffer);
    if( !tokbuf )
    {
        perror("parse_line(): strdup() error");
        return NULL;
    }
    int tokcount = 0;
    if( strtok(tokbuf, whitespace) )
    {
        ++tokcount;
        while( strtok(NULL, whitespace) ) ++tokcount;
    }
    free(tokbuf);

    //Allocate contiguous memory for token pointers and token buffer.
    //That way only one pointer needs to be freed later.
    char** tokp_buf = malloc( sizeof(char**)*(tokcount + 1) + strlen(buffer) + 1);
    if( !tokp_buf )
    {
        perror("parse_line(): error allocating memory for token buffer");
        return NULL;
    }
    tokbuf = (char*)(tokp_buf + tokcount + 1); //Token buffer follows pointer buffer
    strcpy(tokbuf, buffer);

    //Find tokens again, this time filling pointer array.
    size_t i = 0;
    char* tok = strtok(tokbuf, whitespace);
    while(tok)
    {
        tokp_buf[i++] = tok;
        tok = strtok(NULL, whitespace);
    }
    tokp_buf[i] = NULL;

    return tokp_buf;
}

/*
    TODO: documentation
*/
void exec_command(char** argv, int pipe_fd_in, int pipe_fd_out)
{
    const pid_t pid = fork();
    if( pid == -1 ) //Error
    {
        perror("fork()");
        //TODO: better error handling
        exit(EXIT_FAILURE);
    }
    else if( pid == 0 ) //Child
    {
        if( dup2(pipe_fd_in, STDIN_FILENO) == -1 ||
            dup2(pipe_fd_out, STDOUT_FILENO) == -1 )
        {
            perror("Error duplicating file descriptor");
            abort();
        }

        execvp(argv[0], argv);
        perror(argv[0]);
        abort();
    }
    else //Parent
    {
        return;
    }
}

/*
    Wait for child processes
*/
void reap_children(void)
{
    int wstatus;
    while( wait(&wstatus) != -1 );
    //Expect errno == ECHILD when all child processes have been reaped
    if( errno != ECHILD )
    {
        perror("Error waiting for child process");

        if(WIFEXITED(wstatus)) //Should never be encountered in this code block. Included for completness sake
        {
            fprintf(stderr, "Child process exited normally with status code %d\n", WEXITSTATUS(wstatus));
        }
        if(WIFSIGNALED(wstatus))
        {
            fprintf(stderr, "Child process was terminated by signal %d\n", WTERMSIG(wstatus));
            if(WCOREDUMP(wstatus)) fprintf(stderr, "Child process dumped core");
        }
        if(WIFSTOPPED(wstatus))
        {
            fprintf(stderr, "Child process was stopped by signal %d\n", WSTOPSIG(wstatus));
        }
        if(WIFCONTINUED(wstatus))
        {
            fprintf(stderr, "Child process was continued by SIGCONT\n");
        }

        if( kill(0, SIGABRT) == -1 )
        {
            perror("Kill process group error");
        }
        abort();
    }
}
