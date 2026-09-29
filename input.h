/*
    TODO: file header
*/
#ifndef INPUT_H
#define INPUT_H
#include <stdio.h>

#include "debug.h"

/*
    TODO: documentation
*/
//TODO: rename -> parse_input()?
char*** get_command_lines(FILE* infile, size_t* out_nrof_lines);

/*
    TODO: documentation
*/
void free_command_lines(char*** arr);

#endif //INPUT_H
