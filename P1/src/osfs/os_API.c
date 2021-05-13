#include "os_API.h"


// ----------- General Functions ----------- //

// Mount disk into a specific partition
void os_mount(char* diskname, int partition) {
    // TODO:
}

// Display bitmap for current partition
void os_bitmap(unsigned num) {
    // TODO:
}

// Check if file exists
void os_exists(char* filename) {
    // TODO:
}

// List all files in current partition
void os_ls() {
    // TODO:
}

// ----------- MBT Functions -------------- //

// Display valid partitions
void os_mbt() {
    // TODO:
}

// Create new partition
void os_create_partition(int id, int size) {
    // TODO:
}

// Delete partition
void os_delete_partition(int id) {
    // TODO:
}

// Delete all partitions
void reset_mbt() {
    // TODO:
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
