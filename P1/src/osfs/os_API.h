#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#pragma once


// Error handling
typedef enum error {
    NoError,                // no errors stored
    DiskNotFound,           // invalid path to disk
    NoSpaceAvailable,       // not enough space available for operation
    NoBlocksAvailable,      // no blocks available for use in current partition
    FileNotFound,           // file not found
    FileExists,             // file already exists
    MaxFileSizeReached,     // [w] max file size reached
    PartitionNotFound,      // partition not in disk
    PartitionOutOfRange,    // partition id/size out of range
    PartitionExists,        // partition id already in use
    NoDirectoryEntry,       // no directory entries available
    InvalidBitmapIndex,     // bitmap index out of range
    InvalidFileName,        // filename length > 28
    InvalidFileMode,        // file open mode != ('w', 'r')
    InvalidBytesNumber,     // negative number of bytes to read/write
    BytesExceeded,          // [w] nbytes is larger than effectively read bytes
} Error;

extern Error OS_ERROR;

// Struct to manage files
typedef struct os_file {
    char mode;               // Mode (Read/Write)
    unsigned partition_pos;  // Absolute ID for partition (root directory)
    unsigned partition_size; // Size of partition (in blocks)
    unsigned index_block;    // Index block for this file
    unsigned bytes;          // Bytes currently read/written
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

// Create new partition
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

// Display free & used blocks
int* count_bitmap_blocks(char* bitmap, int limit);

// Sort valid partitions according to order in disk
void sort_mbt(unsigned* partitions, unsigned* sizes, int n);

// Converts unsigned between little endian & big endian
unsigned to_big_endian(unsigned n, int n_bytes);

// Converts unsigned long between little endian & big endian
unsigned long to_big_endian_long(unsigned long n, int n_bytes);

// Reserve new block for data
unsigned get_free_block(unsigned partition_start, unsigned partition_size, FILE* file);

// Get global variables
char* get_diskname();

// Free memory for global variables
void os_unmount();

// ----------- Bonus ----------- //
void os_strerror(Error err);
