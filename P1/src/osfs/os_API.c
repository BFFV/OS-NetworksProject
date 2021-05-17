#include "os_API.h"

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
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");

    // 1024 skip MBT, partition_start * 2048 to partition, 2048 skips directory
    fseek(file, 1024 + partition_info[0] * 2048 + 2048, SEEK_SET);
    printf("Found Partition ABS ID: %u\n", partition_info[0]);
    int bitmap_count = partition_info[1] / 16384;
    char* buffer = calloc(2048, sizeof(char));

    // Display all bitmaps
    if (!num) {
        for (int bitmap = 0; bitmap < bitmap_count; bitmap++) {
            fread(buffer, 2048, 1, file);

            // char* ptr;
            //partition_start = (unsigned) strtoul(buffer[0], &ptr, 2);

            // Free memory and return start pointer
            free(buffer);
            fclose(file);

        }
    } else {
        //TODO: Single bitmap
    }

    // Memory cleaning
    fclose(file);
    free(partition_info);

    // TODO: Bonus: Tirar error si no existe la particion
    // fprintf(stderr, )
}

// Check if file exists
int os_exists(char* filename) {
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");
    char* buffer = calloc(32, sizeof(char));

    // 1024 skip MBT, partition_start * 2048 to partition directory
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    for (int entry = 0; entry < 64; entry++) {
        fseek(file, 32, SEEK_CUR);
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
        fseek(file, 32, SEEK_CUR);
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
        fseek(file, entry * 8, SEEK_SET);
        fread(buffer, 1, 8, file);

        // check if block is valid
        unsigned is_valid = (unsigned) buffer[0] >> 7;
        int partition_id = (int)(buffer[0] << 1) >> 1;

        if (is_valid) {
            printf("PID: %d\n", partition_id);
        }
    }

    // memory clean
    free(buffer);
    fclose(file);
}

// Create new partition
void os_create_partition(int id, int size) {
    // TODO:
}

// Delete partition
void os_delete_partition(int id) {
    char* buffer = calloc(8, sizeof(char));
    FILE* file = fopen(disk_path, "rb+");

    for (int entry = 0; entry < 128; entry++){
        fseek(file, entry * 8, SEEK_SET);
        fread(buffer, 1, 8, file);

        // check for block validity and id
        unsigned is_valid = (unsigned) buffer[0] >> 7;
        int partition_id = (int)(buffer[0] << 1) >> 1;

        char valid[1];
        valid[0] = (buffer[0] << 1) >> 1;

        // Change valid bit to zero if it is valid
        if (is_valid && partition_id == id) {
            fseek(file, 0, SEEK_CUR);
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

    for (int entry = 0; entry < 128; entry++){
        fseek(file, entry * 8, SEEK_SET);
        fread(buffer, 1, 8, file);

        // Change valid bit to zero if it is valid
        char valid[1];
        valid[0] = (buffer[0] << 1) >> 1;

        fseek(file, 0, SEEK_CUR);
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
        fseek(file, entry * 8, SEEK_SET);
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

// Count free blocks and used blocks
unsigned* count_bitmap_blocks(unsigned bitmap_hex) {
    unsigned used_blocks = 0;
    unsigned free_blocks = 0;
    while (bitmap_hex) {
        unsigned is_used = bitmap_hex & 1;
        if (is_used) {
            used_blocks++;
        } else {
            free_blocks++;
        }
        bitmap_hex >>= 1;
    }
    unsigned blocks[2];
    blocks[0] = used_blocks;
    blocks[1] = free_blocks;
    return blocks;
}

// Free memory for global variables
void clean_vars() {
    if (disk_path != NULL) {
        free(disk_path);
    }
}
