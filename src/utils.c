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

void swap(void *a, void *b, size_t l)
{
    // REPLACE memcpy to memccpy function
    void *temp = calloc(1, l); 
    memcpy(temp, a, l);
    memcpy(a, b, l);
    memcpy(b, temp, l);
    free(temp);
}

