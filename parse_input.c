/** @file
    Parse command lines from an input file

    @author Aleksander Zdunek
    @date 2026-11-01
*/
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "parse_input.h"

//----------------------------- Internal functions -----------------------------

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

//----------------------------- External functions -----------------------------

char*** parse_input(FILE* infile, size_t* out_nrof_lines)
{
    size_t arr_len = 0;
    char*** arr = NULL;
    char linebuf[1025];
    while(fgets(linebuf, sizeof(linebuf), infile))
    {
        char*** tmp = realloc(arr, (arr_len++) * sizeof(arr[0]));
        if(!tmp)
        {
            fprintf(stderr, "%s:%d:%s(): realloc error\n", __FILE__, __LINE__, __func__);
            free_command_lines(arr, arr_len);
            return NULL;
        }
        arr = tmp;

        if(!(arr[arr_len - 1] = parse_line(linebuf)))
        {
            free_command_lines(arr, arr_len);
            return NULL;
        }
    }
    if(ferror(infile))
    {
        perror("Error reading command lines");
        free_command_lines(arr, arr_len);
        return NULL;
    }

    assert(out_nrof_lines);
    *out_nrof_lines = arr_len;
    return arr;
}

void free_command_lines(char*** arr, size_t count)
{
    if(!arr) return;
    for(size_t i = 0; i < count; ++i)
    {
        //Don't need to free each string individually because we made sure in
        //parse_lines() that the strings are held in the same contigiously
        //allocated block of memory.
        free(arr[i]);
    }
    free(arr);
}
