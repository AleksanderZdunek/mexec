#include <string.h>
#include <stdlib.h>
#include "input.h"

/*
    Parse a string into whitespace-separated tokens.

    @param buffer Pointer to null-terminated string to parse.

    @return Pointer to array of token pointers. This pointer should be freed.
        Last pointer in array is NULL.
*/
static char** parse_line(const char* buffer)
{
    const char* const whitespace = " \f\n\r\t\v";
    buffer += strspn(buffer, whitespace); //Ignore leading whitespace

    //Count number of tokens to make it easier to allocate memory for token pointers
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

char*** get_command_lines(FILE* infile, size_t* out_nrof_lines)
{
    //TODO: remove null termination
    //TODO: assert out_nrof_lines not null
    size_t arr_len = 1;
    char*** arr = malloc(arr_len * sizeof(char***));
    if(!arr)
    {
        DEBUG_PRINT("malloc error");
        return NULL;
    }
    *arr = NULL;

    char linebuf[1025];
    while(fgets(linebuf, sizeof(linebuf), infile))
    {
        char*** tmp = realloc(arr, (arr_len + 1) * sizeof(arr[0]));
        if(!tmp)
        {
            DEBUG_PRINT("realloc error");
            free_command_lines(arr);
            return NULL;
        }
        arr = tmp;

        if(!(arr[arr_len - 1] = parse_line(linebuf)))
        {
            free_command_lines(arr);
            return NULL;
        }
        arr[arr_len++] = NULL;
    }
    if(ferror(infile))
    {
        perror("Error reading command lines");
        free_command_lines(arr);
        return NULL;
    }

    if(out_nrof_lines) *out_nrof_lines = arr_len - 1;
    return arr;
}

void free_command_lines(char*** arr)
{
    if(!arr) return;
    for(char*** p = arr; *p; ++p)
    {
        //Don't need to free each string individually because parse_lines() has
        //made sure the strings get deallocated together with the string pointer
        //array.
        free(*p);
    }
    free(arr);
}
