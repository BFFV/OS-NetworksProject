#include "file_manager.h"


// Write image data
void write_img(char* filename, char* data, int size) {
    FILE* output = fopen(filename, "wb");
    fwrite(data, 1, size, output);
    fclose(output);
}

// Create directory if non-existant
void create_dir(char* name) {
    mkdir(name, 0777);
}
