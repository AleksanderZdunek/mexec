#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

char** parse_line(const char* buffer);

int main(int argc, char* argv[])
{
    FILE* infile = stdin;
    if( 2 == argc )
    {
        infile = fopen(argv[1], "r");
        if( !infile )
        {
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    }
    else if( argc > 2 )
    {
        fprintf(stderr, "Too many arguments\nUsage: mexec <filename>\n");
        exit(EXIT_FAILURE);
    }

    char linebuf[1025];
    while( fgets(linebuf, sizeof(linebuf), infile) )
    //TODO: error checking
    {
        printf("%s", linebuf);
    }

    //TODO: close infile

    return 0;
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
