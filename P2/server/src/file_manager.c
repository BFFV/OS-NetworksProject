#include "file_manager.h"


// Read image data
int read_img(char* filename, char* data) {
    FILE* input = fopen(filename, "rb");
    char* buffer = calloc(4096, sizeof(char));
    int total_bytes = 0;
    size_t bytes_read = 0;
    bool reading = true;
    while (reading) {
        bytes_read = fread(buffer, 1, 4096, input);
        memcpy(data + total_bytes, buffer, bytes_read);
        total_bytes += bytes_read;
        if (bytes_read < 4096) {
            reading = false;
        }
    }
    free(buffer);
    fclose(input);
    return total_bytes;
}
