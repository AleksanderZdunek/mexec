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

/*
    TODO: documentation
*/
char*** parse_input(FILE* infile, size_t* out_nrof_lines);

/*
    TODO: documentation
*/
void free_command_lines(char*** arr, size_t count);

#endif //PARSE_INPUT_H
