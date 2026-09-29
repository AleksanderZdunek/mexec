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
char*** get_command_lines(FILE* infile);

/*
    TODO: documentation
*/
void free_command_lines(char*** arr);

#endif //INPUT_H
