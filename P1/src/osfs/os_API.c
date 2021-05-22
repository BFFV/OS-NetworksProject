#include "os_API.h"

#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define DEFAULT "\e[0m"

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
    fseek(file, 1024 + (partition_info[0] + 1) * 2048, SEEK_SET);
    fpos_t position;
    fgetpos(file, &position);
    int bitmap_count = ceil((double) partition_info[1] / 16384);
    unsigned* buffer = calloc(1, sizeof(unsigned));
    char* bitmap_block = calloc(2048, sizeof(char));

    // Display bitmaps
    if (!num) {
        for (int bitmap = 0; bitmap < bitmap_count; bitmap++) {
            fsetpos(file, &position);
            fseek(file, 2048 * bitmap, SEEK_CUR);

            // Convert bytes to hex
            fprintf(stderr, GRN "\n0x\n");
            for (int byte = 0; byte < 2048; byte++) {
                fread(buffer, 1, 1, file);
                unsigned current_byte = *buffer;
                fprintf(stderr, GRN "%X", current_byte >> 4);
                fprintf(stderr, GRN "%X", current_byte & 15);
                if (!((byte + 1) % 64)) {
                    fprintf(stderr, "\n");
                }
            }
            fsetpos(file, &position);
            fseek(file, 2048 * bitmap, SEEK_CUR);
            fread(bitmap_block, 2048, 1, file);
            count_bitmap_blocks(bitmap_block);
        }
    } else {
        fseek(file, 2048 * (num - 1), SEEK_CUR);

        // Convert bytes to hex
        fprintf(stderr, GRN "\n0x\n");
        for (int byte = 0; byte < 2048; byte++) {
            fread(buffer, 1, 1, file);
            unsigned current_byte = *buffer;
            fprintf(stderr, GRN "%X", current_byte >> 4);
            fprintf(stderr, GRN "%X", current_byte & 15);
            if (!((byte + 1) % 64)) {
                fprintf(stderr, "\n");
            }
        }
        fsetpos(file, &position);
        fseek(file, 2048 * (num - 1), SEEK_CUR);
        fread(bitmap_block, 2048, 1, file);
        count_bitmap_blocks(bitmap_block);
    }
    fprintf(stdout, DEFAULT "\n");

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
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* pos = calloc(1, sizeof(unsigned));
    unsigned* size = calloc(1, sizeof(unsigned));

    FILE* file = fopen(disk_path, "rb");

    printf(">> Valid Partitions\n\n");
    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);
        fread(partition_header, 1, 1, file);

        // check if block is valid
        unsigned is_valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        if (is_valid) {
            fread(pos, 3, 1, file);
            fread(size, 4, 1, file);
            printf("PID: %u\n", partition_id);
            printf("  - Size: %u @ Pos: %u \n\n", *size, *pos);
        }
    }

    // Memory clean
    free(partition_header);
    free(size);
    free(pos);
    fclose(file);
}

// Create new partitions
void os_create_partition(int id, int size) {
    // TODO: ID not in range, invalid size

    unsigned* partitions = malloc(128 * sizeof(unsigned));
    unsigned* sizes = malloc(128 * sizeof(unsigned));
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* abs_id = calloc(1, sizeof(unsigned));
    unsigned* partition_size = calloc(1, sizeof(unsigned));

    int valid_partitions = 0;
    int new_entry = -1;
    FILE* file = fopen(disk_path, "rb");

    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);
        fread(partition_header, 1, 1, file);

        // Get entry information
        unsigned valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Checks valid bit
        if (valid) {

            if (id == partition_id) {
                //TODO: Error existing id
                printf("F\n");
            }

            // Get absolute position for partition
            fread(abs_id, 3, 1, file);
            partitions[valid_partitions] = *abs_id;

            // Get size for partition
            fread(partition_size, 4, 1, file);
            sizes[valid_partitions] = *partition_size;

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
            free_space = 2097152 - previous_end;
        } else {
            free_space = partitions[p] - previous_end;
            previous_end = partitions[p] + sizes[p];
        }
        if ((free_space >= size) && searching) {
            FILE* file = fopen(disk_path, "rb+");
            fseek(file, new_entry * 8, SEEK_SET);

            unsigned valid = 128 | id;
            fwrite(&valid, 1, 1, file);
            fwrite(&previous_end, 3, 1, file);
            fwrite(&size, 4, 1, file);

            // Erase previous directory & bitmaps from partition
            fseek(file, 1024 + previous_end * 2048, SEEK_SET);
            for (int byte = 0; byte < 2048; byte++) {
                unsigned current_byte = 0;
                fwrite(&current_byte, 1, 1, file);
            }
            int bitmap_count = ceil((double) size / 16384);
            unsigned header = 128;
            for (int block = 0; (block < bitmap_count) && (block < 7); block++) {
                header = 128 | (header >> 1);
            }
            fwrite(&header, 1, 1, file);
            for (int byte = 1; byte < bitmap_count * 2048; byte++) {
                unsigned current_byte = 0;
                if ((bitmap_count == 8) && (byte == 1)) {
                    current_byte = 128;
                }
                fwrite(&current_byte, 1, 1, file);
            }
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
    free(partition_header);
    free(abs_id);
    free(partition_size);
}

// Delete partition
void os_delete_partition(int id) {
    unsigned* partition_header = calloc(1, sizeof(unsigned));

    FILE* file = fopen(disk_path, "rb+");
    fpos_t position;

    for (int entry = 0; entry < 128; entry++){
        fseek(file, entry * 8, SEEK_SET);

        fgetpos(file, &position);
        fread(partition_header, 1, 1, file);

        // check for block validity and id
        unsigned is_valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Change valid bit to zero if it is valid
        unsigned valid = 0;
        if (is_valid && partition_id == id) {
            fsetpos(file, &position);
            fwrite(&valid, 1, 1, file);
        }
    }

    // memory clean
    fclose(file);
}

// Delete all partitions
void reset_mbt() {
    FILE* file = fopen(disk_path, "rb+");

    for (int entry = 0; entry < 128; entry++){
        fseek(file, entry * 8, SEEK_SET);

        // Change valid bit to zero if it is valid
        unsigned valid = 0;
        fwrite(&valid, 1, 1, file);
    }

    // memory clean
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
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* abs_id = calloc(1, sizeof(unsigned));
    unsigned* partition_size = calloc(1, sizeof(unsigned));
    FILE* file = fopen(disk_path, "rb");

    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);
        fread(partition_header, 1, 1, file);

        // Get entry information
        unsigned valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Checks valid bit and if block represents current partition
        if (valid && (partition_id == current_partition)) {

            // Get absolute position for partition
            fread(abs_id, 3, 1, file);
            partition_info[0] = *abs_id;

            // Get size for partition
            fread(partition_size, 4, 1, file);
            partition_info[1] = *partition_size;

            // Free memory and return start pointer
            free(partition_header);
            free(abs_id);
            free(partition_size);
            fclose(file);
            return partition_info;
        }
    }

    free(partition_header);
    free(abs_id);
    free(partition_size);
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
    fprintf(stderr, GRN "Free Blocks: %u\n\n", free_blocks);
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
