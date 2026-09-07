#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

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


    return 0;
}
