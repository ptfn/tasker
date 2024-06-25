#include "tasker.h"

/* Open File */
FILE *open_file(char *fname, char *mode)
{
    FILE *file;
    if ((file = fopen(fname, mode)) == NULL) {
        perror("Error occured while opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}

