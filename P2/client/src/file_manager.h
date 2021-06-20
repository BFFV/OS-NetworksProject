#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>


// Write image data
void write_img(char* filename, char* data, int size);

// Create directory if non-existant
void create_dir(char* name);
