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

/* Swap Struct Byte */
void swap(void *a, void *b, size_t size)
{
    // REPLACE memcpy to memccpy function
    void *temp = calloc(1, size); 
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
    free(temp);
}

