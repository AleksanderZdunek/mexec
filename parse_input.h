/** @file
    Parse command lines from an input file

    @author Aleksander Zdunek
    @date 2026-11-01
*/
#ifndef PARSE_INPUT_H
#define PARSE_INPUT_H
#include <stdio.h>

//TODO: remove
#include "debug.h"

/**
    Parse an input file where each line represents a command line.

    @param infile Input stream
    @param[out] line_count Pointer to memory that on success will hold the
        length of the returned array.

    @return On success returns a pointer to an array of pointers to parsed
        command lines. Each command line is a null-terminated array of strings
        for a command and arguments.
        The return value should be freed using free_command_line().
        On errror returns NULL.
*/
char*** parse_input(FILE* infile, size_t* line_count);

/**
    Free the array and command lines returned from parse_input()

    @param arr Pointer to array of parsed command lines
    @param count Number of command lines in array
*/
void free_command_lines(char*** arr, size_t count);

#endif //PARSE_INPUT_H
