#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#pragma once


// Manage files
typedef struct os_file {
    int file_id;
} osFile;

// ----------- General Functions ----------- //

// Mount disk into a specific partition
void os_mount(char* diskname, int partition);

// Display bitmap for current partition
void os_bitmap(unsigned num);

// Check if file exists
int os_exists(char* filename);

// List all files in current partition
void os_ls();

// ----------- MBT Functions -------------- //

// Display valid partitions
void os_mbt();

// Creatchar* ptr;e new partition
void os_create_partition(int id, int size);

// Delete partition
void os_delete_partition(int id);

// Delete all partitions
void reset_mbt();

// ----------- File Management Functions ----- //

// Open file
osFile* os_open(char* filename, char mode);

// Read file
int os_read(osFile* file_desc, void* buffer, int nbytes);

// Write file
int os_write(osFile* file_desc, void* buffer, int nbytes);

// Close file
int os_close(osFile* file_desc);

// Remove file
int os_rm(char* filename);

// ----------- Utils ----------- //

// Find location & size for current partition
unsigned* find_partition();

// Count free blocks and used blocks
unsigned* count_bitmap_blocks(unsigned bitmap_hex);

// Free memory for global variables
void clean_vars();
