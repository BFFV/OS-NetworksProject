#include "os_API.h"
#include <stdbool.h>

#define RED "\e[0;31m"
#define GRN "\e[0;32m"

// ----------- Global Variables ----------- //

char* disk_path;  // Path to current disk
int current_partition;  // Current partition where the disk is mounted

// ----------- General Functions ----------- //

// Mount disk into a specific partition
void os_mount(char* diskname, int partition) {
    clean_vars();
    disk_path = calloc(strlen(diskname) + 1, sizeof(char));
    strcpy(disk_path, diskname);
    current_partition = partition;
}

// Display bitmap for current partition
void os_bitmap(unsigned num) {
    // TODO: Bonus: partition not found
    unsigned* partition_info = find_partition();

    //unsigned partition_info[2] = {1, 16384 * 2};

    FILE* file = fopen(disk_path, "rb");

    // 1024 skip MBT, partition_start * 2048 to partition, 2048 skips directory
    fseek(file, 1024 + partition_info[0] * 2048 + 2048, SEEK_SET);
    int bitmap_count = partition_info[1] / 16384;
    char* buffer = calloc(2048, sizeof(char));

    // Display bitmaps
    if (!num) {
        for (int bitmap = 0; bitmap < bitmap_count; bitmap++) {
            fread(buffer, 1, 2048, file);

            // Convert bytes to hex
            fprintf(stderr, GRN "\n0x\n");
            for (int byte = 0; byte < 2048; byte++) {
                char current_byte = buffer[byte];
                fprintf(stderr, GRN "%x", current_byte >> 4);
                fprintf(stderr, GRN "%x", (current_byte << 4) >> 4);
                if (!((byte + 1) % 64)) {
                    fprintf(stderr, "\n");
                }
            }
            count_bitmap_blocks(buffer);
        }
    } else {
        fseek(file, 2048 * (num - 1), SEEK_CUR);
        fread(buffer, 1, 2048, file);

        // Convert bytes to hex
        fprintf(stderr, GRN "\n0x\n");
        for (int byte = 0; byte < 2048; byte++) {
            char current_byte = buffer[byte];
            fprintf(stderr, GRN "%x", current_byte >> 4);
            fprintf(stderr, GRN "%x", (current_byte << 4) >> 4);
            if (!((byte + 1) % 64)) {
                fprintf(stderr, "\n");
            }
        }
        count_bitmap_blocks(buffer);
    }

    // Memory cleaning
    free(buffer);
    free(partition_info);
    fclose(file);
}

// Check if file exists
int os_exists(char* filename) {
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");
    char* buffer = calloc(32, sizeof(char));

    // 1024 skip MBT, partition_start * 2048 to partition directory
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    for (int entry = 0; entry < 64; entry++) {
        fread(buffer, 1, 32, file);
        unsigned is_valid = (unsigned) buffer[0];
        char current_filename[28];
        for (int c = 4; c < 32; c++) {
            current_filename[c - 4] = buffer[c];
        }
        if (is_valid && strcmp(current_filename, filename)) {
            free(buffer);
            fclose(file);
            return 1;
        }
    }
    free(buffer);
    fclose(file);
    return 0;
}

// List all files in current partition
void os_ls() {
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");
    char* buffer = calloc(32, sizeof(char));

    // 1024 skip MBT, partition_start * 2048 to partition directory
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    for (int entry = 0; entry < 64; entry++) {
        fread(buffer, 1, 32, file);

        // check if block is valid
        unsigned is_valid = (unsigned) buffer[0];
        if (is_valid) {
            // get filename from buffer entry
            char file_name[28];
            for (int idx = 0; idx < 28; idx++){
                file_name[idx] = buffer[idx + 4];
            }
            // print filename
            printf("File: %s\n", file_name);
        }
    }

    // memory cleaning
    free(buffer);
    fclose(file);
}

// ----------- MBT Functions -------------- //

// Display valid partitions
void os_mbt() {
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb");
    printf(">> Valid Partitions\n\n");

    for (int entry = 0; entry < 128; entry++){
        fread(buffer, 1, 8, file);

        // check if block is valid
        unsigned is_valid = (unsigned) buffer[0] >> 7;
        int partition_id = (int)(buffer[0] << 1) >> 1;

        if (is_valid) {
            printf("PID: %d\n", partition_id);
        }
    }

    // Memory clean
    free(buffer);
    fclose(file);
}

// Create new partitions
void os_create_partition(int id, int size) {
    // TODO: ID not in range, invalid size
    unsigned* partitions = malloc(128 * sizeof(unsigned));
    unsigned* sizes = malloc(128 * sizeof(unsigned));
    int valid_partitions = 0;
    int new_entry = -1;
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb");

    for (int entry = 0; entry < 128; entry++) {
        fread(buffer, 1, 8, file);

        // Get entry information
        unsigned valid = buffer[0] >> 7;
        int partition_id = (int) (buffer[0] << 1) >> 1;

        // Checks valid bit
        if (valid) {

            if (id == partition_id) {
                //TODO: Error existing id
                printf("F");
            }

            // Get absolute position for partition
            char abs_id[3];
            for (int index = 1; index < 4; index++) {
                abs_id[index - 1] = buffer[index];
            }
            char* ptr;
            partitions[valid_partitions] = (unsigned) strtoul(abs_id, &ptr, 2);

            // Get size for partition
            char partition_size[4];
            for (int index = 4; index < 8; index++) {
                partition_size[index - 4] = buffer[index];
            }
            sizes[valid_partitions] = (unsigned) strtoul(partition_size, &ptr, 2);

            valid_partitions++;
        } else if (new_entry == -1) {
            new_entry = entry;
        }
    }

    fclose(file);
    sort_mbt(partitions, sizes, valid_partitions);
    bool searching = true;

    unsigned previous_end = 0;
    for (int p = 0; p <= valid_partitions; p++) {
        unsigned free_space;
        if (p == valid_partitions) {
            free_space = 2097151 - previous_end;
        } else {
            free_space = partitions[p] - previous_end;
            previous_end = partitions[p] + sizes[p];
        }
        if ((free_space >= size) && searching) {
            FILE *file = fopen(disk_path, "rb+");
            fseek(file, new_entry * 8, SEEK_SET);

            unsigned valid = 128 + id;
            fwrite(&valid, 1, 1, file);
            fwrite(&previous_end, 3, 1, file);
            fwrite(&size, 4, 1, file);

            // Erase previous directory & bitmaps from partition
            fseek(file, 1024 + previous_end * 2048, SEEK_SET);
            unsigned empty = 0;
            int bitmap_count = size / 16384;
            fwrite(&empty, (bitmap_count + 1) * 2048, 1, file);
            fclose(file);

            searching = false;
        }
    }

    // TODO: No space
    if (searching) {
        printf("F: No hay espacio\n");
    }

    // Free memory
    free(partitions);
    free(sizes);
    free(buffer);
}

// Delete partition
void os_delete_partition(int id) {
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb+");
    fpos_t position;

    for (int entry = 0; entry < 128; entry++){
        fgetpos(file, &position);
        fread(buffer, 1, 8, file);

        // check for block validity and id
        unsigned is_valid = (unsigned) buffer[0] >> 7;
        int partition_id = (int)(buffer[0] << 1) >> 1;

        unsigned valid = 0;

        // Change valid bit to zero if it is valid
        if (is_valid && partition_id == id) {
            fsetpos(file, &position);
            fwrite(&valid, 1, 1, file);
        }
    }

    // memory clean
    free(buffer);
    fclose(file);
}

// Delete all partitions
void reset_mbt() {
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb+");
    fpos_t position;

    for (int entry = 0; entry < 128; entry++){
        fgetpos(file, &position);
        fread(buffer, 1, 8, file);

        // Change valid bit to zero if it is valid
        unsigned valid = 0;

        fsetpos(file, &position);
        fwrite(&valid, 1, 1, file);
    }

    // memory clean
    free(buffer);
    fclose(file);
}

// ----------- File Management Functions ----- //

// Open file
osFile* os_open(char* filename, char mode) {
    // TODO:
}

// Read file
int os_read(osFile* file_desc, void* buffer, int nbytes) {
    // TODO:
}

// Write file
int os_write(osFile* file_desc, void* buffer, int nbytes) {
    // TODO:
}

// Close file
int os_close(osFile* file_desc) {
    // TODO:
}

// Remove file
int os_rm(char* filename) {
    // TODO:
}

// ----------- Utils ----------- //

// Find location for current partition
unsigned* find_partition() {
    unsigned* partition_info = malloc(2 * sizeof(unsigned));
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb");
    for (int entry = 0; entry < 128; entry++) {
        fread(buffer, 1, 8, file);

        // Get entry information
        unsigned valid = buffer[0] >> 7;
        int partition_id = (int) (buffer[0] << 1) >> 1;

        // Checks valid bit and if block represents current partition
        if (valid && (partition_id == current_partition)) {

            // Get absolute position for partition
            char abs_id[3];
            for (int index = 1; index < 4; index++) {
                abs_id[index - 1] = buffer[index];
            }
            char* ptr;
            partition_info[0] = (unsigned) strtoul(abs_id, &ptr, 2);

            // Get size for partition
            char partition_size[4];
            for (int index = 4; index < 8; index++) {
                partition_size[index - 4] = buffer[index];
            }
            partition_info[1] = (unsigned) strtoul(partition_size, &ptr, 2);

            // Free memory and return start pointer
            free(buffer);
            fclose(file);
            return partition_info;
        }
    }

    free(buffer);
    fclose(file);
    // TODO: Partición no encontrada
    return NULL;
}

// Display free & used blocks
void count_bitmap_blocks(char* bitmap) {
    int used_blocks = 0;
    int free_blocks = 0;
    for (int byte = 0; byte < 2048; byte++) {
        char current = bitmap[byte];
        for (int bit = 0; bit < 8; bit++) {
            unsigned is_used = current & 1;
            if (is_used) {
                used_blocks++;
            } else {
                free_blocks++;
            }
            current >>= 1;
        }
    }
    fprintf(stderr, GRN "\n\nUsed Blocks: %u\n", used_blocks);
    fprintf(stderr, GRN "Free Blocks: %u\n", free_blocks);
}

// Sort valid partitions according to order in disk
void sort_mbt(unsigned* partitions, unsigned* sizes, int n){
    for (int idx = 0; idx < n; idx++){
        for (int jdx = idx; jdx < n; jdx++) {
            if (partitions[jdx] < partitions[idx]) {

                // Sort partitions array
                unsigned aux = partitions[idx];
                partitions[idx] = partitions[jdx];
                partitions[jdx] = aux;

                // Make sizes consistent with partitions order
                aux = sizes[idx];
                sizes[idx] = sizes[jdx];
                sizes[jdx] = aux;
            }
        }
    }
}

// Free memory for global variables
void clean_vars() {
    if (disk_path != NULL) {
        free(disk_path);
    }
}
