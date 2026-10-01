/** @file
    Parse command lines from an input file

    @author Aleksander Zdunek
    @date 2026-11-01
*/
#ifndef INPUT_H
#define INPUT_H
#include <stdio.h>

//TODO: remove
#include "debug.h"

/*
    TODO: documentation
*/
//TODO: rename -> parse_input()?
char*** get_command_lines(FILE* infile, size_t* out_nrof_lines);

/*
    TODO: documentation
*/
void free_command_lines(char*** arr, size_t count);

#endif //INPUT_H
