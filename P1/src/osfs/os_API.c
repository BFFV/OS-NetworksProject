#include "os_API.h"

// ----------- Global Variables ----------- //

char* disk_path; // Path to current disk
bool init = false; // Disk mounted initially
int current_partition; // Current partition where the disk is mounted
Error OS_ERROR; // Error handling

// ----------- General Functions ----------- //

// Mount disk into a specific partition
void os_mount(char* diskname, int partition) {

    // Check for existing disk
    FILE* file = fopen(diskname, "rb");
    if (file == NULL) {
        OS_ERROR = DiskNotFound;
        return;
    }
    fclose(file);

    // Default mount
    if (!init) {
        disk_path = calloc(strlen(diskname) + 1, sizeof(char));
        strcpy(disk_path, diskname);
        init = true;

    } else if (partition < 0 || partition > 127) { // Partition out of range
        OS_ERROR = PartitionOutOfRange;

    } else {
        os_unmount();
        disk_path = calloc(strlen(diskname) + 1, sizeof(char));
        strcpy(disk_path, diskname);
        int old_partition = current_partition;
        current_partition = partition;

        // PartitionNotFound: reset mounted partition
        unsigned* partition_info = find_partition();
        if (partition_info == NULL) {
            current_partition = old_partition;
            return;
        }
        free(partition_info);

        // Success message
        fprintf(stdout, GRN "\n>>> The partition [%d] has been successfully mounted!\n\n", partition);
        fprintf(stdout, DEFAULT "");
    }
}

// Display bitmap for current partition
void os_bitmap(unsigned num) {

    // Get partition
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");

    // 1024 skips MBT, partition_start * 2048 skips to partition, 2048 skips to bitmaps
    fseek(file, 1024 + (partition_info[0] + 1) * 2048, SEEK_SET);
    fpos_t position; // Holds position after partition directory
    fgetpos(file, &position);
    int bitmap_count = ceil((double) partition_info[1] / 16384);

    // Invalid bitmap index
    if (num > bitmap_count || num < 0) {
        OS_ERROR = InvalidBitmapIndex;
        free(partition_info);
        fclose(file);
        return;
    }

    // Assign memory buffers
    unsigned* buffer = calloc(1, sizeof(unsigned));
    char* bitmap_block = calloc(2048, sizeof(char));
    unsigned total_used = 0;
    unsigned total_free = 0;

    // Display bitmaps
    for (int bitmap = 0; bitmap < bitmap_count; bitmap++) {
        fsetpos(file, &position);
        fseek(file, 2048 * bitmap, SEEK_CUR);

        // Convert bytes to hex & count free blocks
        fread(bitmap_block, 2048, 1, file);
        int bits_to_read = 16384;
        int remaining = partition_info[1] % 16384;
        if ((bitmap == bitmap_count - 1) && (remaining)) {
            bits_to_read = remaining;
        }
        int* block_data = count_bitmap_blocks(bitmap_block, bits_to_read);
        total_used += block_data[0];
        total_free += block_data[1];
        if ((!num) || (num == bitmap + 1)) {
            fprintf(stderr, GRN "\nBitmap Block %d\n", bitmap + 1);
            fprintf(stderr, GRN "\nUsed Blocks: %u\n", block_data[0]);
            fprintf(stderr, GRN "Free Blocks: %u\n\n", block_data[1]);
            fsetpos(file, &position);
            fseek(file, 2048 * bitmap, SEEK_CUR);
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
        }
        free(block_data);
    }

    fprintf(stderr, GRN "\n\nTotal Used Blocks in Partition: %u\n", total_used);
    fprintf(stderr, GRN "Total Free Blocks in Partition: %u\n\n", total_free);
    fprintf(stdout, DEFAULT "\n");

    // Memory cleaning
    free(buffer);
    free(bitmap_block);
    free(partition_info);
    fclose(file);
}

// Check if file exists
int os_exists(char* filename) {

    // Check for valid filename
    if (strlen(filename) > 28) {
        OS_ERROR = InvalidFileName;
        return 0;
    }

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

        // Correct filename
        if (is_valid && !strcmp(current_filename, filename)) {
            free(buffer);
            free(partition_info);
            fclose(file);
            return 1;
        }
    }

    // Memory cleaning
    free(buffer);
    free(partition_info);
    fclose(file);
    return 0;
}

// List all files in current partition
void os_ls() {
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb");
    unsigned* valid_buffer = calloc(1, sizeof(unsigned));
    char* buffer = calloc(29, sizeof(char));
    printf("\n");

    // 1024 skip MBT, partition_start * 2048 to partition directory
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    for (int entry = 0; entry < 64; entry++) {
        fread(valid_buffer, 1, 1, file);
        fseek(file, 3, SEEK_CUR);
        fread(buffer, 28, 1, file);

        // Valid file
        if (*valid_buffer) {
            printf("%s\n", buffer);
        }
    }

    // Memory cleaning
    free(buffer);
    free(valid_buffer);
    free(partition_info);
    fclose(file);
}

// ----------- MBT Functions -------------- //

// Display valid partitions
void os_mbt() {
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* pos = calloc(1, sizeof(unsigned));
    unsigned* size = calloc(1, sizeof(unsigned));

    FILE* file = fopen(disk_path, "rb");

    printf("\n>> Valid Partitions\n");
    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);
        fread(partition_header, 1, 1, file);

        // Check if block is valid
        unsigned is_valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Valid partition
        if (is_valid) {
            fread(pos, 3, 1, file);
            fread(size, 4, 1, file);
            printf("Partition ID: %u  ", partition_id);
            printf("(Starting Block: %u, Size: %u)\n\n", to_big_endian(*pos, 3), to_big_endian(*size, 4));
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

    // Invalid id/size
    if ((id < 0 || id > 127) || (size < 16384 || size > 131072)) {
        OS_ERROR = PartitionOutOfRange;
        return;
    }

    unsigned* partitions = malloc(128 * sizeof(unsigned));
    unsigned* sizes = malloc(128 * sizeof(unsigned));
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* abs_id = calloc(1, sizeof(unsigned));
    unsigned* partition_size = calloc(1, sizeof(unsigned));

    int valid_partitions = 0;
    int new_entry = -1;
    FILE* file = fopen(disk_path, "rb");

    // Search for currently valid partitions
    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);
        fread(partition_header, 1, 1, file);

        // Get entry information
        unsigned valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Checks valid bit
        if (valid) {

            // ID already exists
            if (id == partition_id) {
                OS_ERROR = PartitionExists;
                free(partitions);
                free(sizes);
                free(partition_header);
                free(abs_id);
                free(partition_size);
                fclose(file);
                return;
            }

            // Get absolute position for partition
            fread(abs_id, 3, 1, file);
            partitions[valid_partitions] = to_big_endian(*abs_id, 3);

            // Get size for partition
            fread(partition_size, 4, 1, file);
            sizes[valid_partitions] = to_big_endian(*partition_size, 4);

            valid_partitions++;
        } else if (new_entry == -1) {
            new_entry = entry;
        }
    }

    fclose(file);
    sort_mbt(partitions, sizes, valid_partitions);
    bool searching = true;

    // Search for free space in disk
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

            // Write MBT entry
            unsigned valid = 128 | id;
            fwrite(&valid, 1, 1, file);
            previous_end = to_big_endian(previous_end, 3);
            fwrite(&previous_end, 3, 1, file);
            previous_end = to_big_endian(previous_end, 3);
            size = to_big_endian(size, 4);
            fwrite(&size, 4, 1, file);
            size = to_big_endian(size, 4);

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

    // Memory clean
    free(partitions);
    free(sizes);
    free(partition_header);
    free(abs_id);
    free(partition_size);

    // No space available
    if (searching) {
        OS_ERROR = NoSpaceAvailable;
        return;
    }

    // Success message
    fprintf(stdout, GRN "\n>>> Partition of ID [%d] and size [%d] has been created!\n\n", id, size);
    fprintf(stdout, DEFAULT "");
}

// Delete partition
void os_delete_partition(int id) {

    // Check for valid partition id
    if (id < 0 || id > 127) {
        OS_ERROR = PartitionOutOfRange;
        return;
    }

    unsigned* partition_header = calloc(1, sizeof(unsigned));

    FILE* file = fopen(disk_path, "rb+");
    fpos_t position;
    bool found = false;

    // Search for partition
    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);

        fgetpos(file, &position);
        fread(partition_header, 1, 1, file);

        // Check for block validity and id
        unsigned is_valid = *partition_header >> 7;
        unsigned partition_id = 128 ^ *partition_header;

        // Change valid bit to zero if it is valid
        unsigned valid = 0;
        if (is_valid && partition_id == id) {
            fsetpos(file, &position);
            fwrite(&valid, 1, 1, file);
            found = true;
        }
    }

    // Memory clean
    free(partition_header);
    fclose(file);

    // Invalid partition id
    if (!found) {
        OS_ERROR = PartitionNotFound;
        return;
    }

    // Success message
    fprintf(stdout, GRN "\n>>> The partition [%d] has been deleted!\n\n", id);
    fprintf(stdout, DEFAULT "");
}

// Delete all partitions
void reset_mbt() {
    FILE *file = fopen(disk_path, "rb+");

    for (int entry = 0; entry < 128; entry++) {
        fseek(file, entry * 8, SEEK_SET);

        // Change valid bit to zero if it is valid
        unsigned valid = 0;
        fwrite(&valid, 1, 1, file);
    }

    // Memory clean
    fclose(file);

    // Success
    fprintf(stdout, GRN "\n>>> The MBT has been successfully reset!\n");
    fprintf(stdout, DEFAULT "");
}

// ----------- File Management Functions ----- //

// Open file
osFile* os_open(char* filename, char mode) {

    // Check for valid filename length
    if (strlen(filename) > 28) {
        OS_ERROR = InvalidFileName;
        return NULL;
    }

    // Invalid mode
    if (mode != 'w' && mode != 'r') {
        OS_ERROR = InvalidFileMode;
        return NULL;
    }

    // Get partition
    FILE* file = fopen(disk_path, "rb+");
    unsigned* partition_info = find_partition();
    unsigned* buffer = calloc(1, sizeof(unsigned));
    char* name_buffer = calloc(28, sizeof(char));

    // 1024 skips MBT, partition_start * 2048 skips to partition
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    fpos_t position; // Holds position for partition directory
    fgetpos(file, &position);

    // Search for file with the same name
    int new_entry = -1;
    bool file_found = false;
    unsigned index_pos = 0;
    for (int entry = 0; entry < 64; entry++) {
        fsetpos(file, &position);
        fseek(file, entry * 32, SEEK_CUR);
        *buffer = 0;
        fread(buffer, 1, 1, file);
        if (*buffer) {
            fread(buffer, 3, 1, file);
            index_pos = to_big_endian(*buffer, 3);
            fread(name_buffer, 1, 28, file);
            file_found = !strcmp(name_buffer, filename);

            // File found
            if (file_found) {
                break;
            }
        } else if (new_entry == -1) {
            new_entry = entry;
        }
    }

    // Memory cleaning
    free(name_buffer);

    // Create osFile for reading
    if (mode == 'r' && file_found) {
        osFile* stream = malloc(sizeof(osFile));
        stream->mode = mode;
        stream->partition_pos = partition_info[0];
        stream->partition_size = partition_info[1];
        stream->index_block = index_pos;
        stream->bytes = 0;

        // Free memory and return osFile
        free(partition_info);
        free(buffer);
        fclose(file);
        return stream;

    } else if (mode == 'r') { // File does not exist
        OS_ERROR = FileNotFound;
        free(partition_info);
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Create osFile for writing
    if (mode == 'w' && !file_found && new_entry >= 0) {

        // Reserve block for index
        unsigned free_block = get_free_block(partition_info[0], partition_info[1], file);
        if (!free_block) {
            free(partition_info);
            free(buffer);
            fclose(file);
            return NULL;
        }

        // Create directory entry
        fsetpos(file, &position);
        fseek(file, 32 * new_entry, SEEK_CUR);
        unsigned valid = 1;
        fwrite(&valid, 1, 1, file);
        free_block = to_big_endian(free_block, 3);
        fwrite(&free_block, 3, 1, file);
        free_block = to_big_endian(free_block, 3);
        for (int byte = 0; byte < strlen(filename); byte++) {
            fwrite(&filename[byte], 1, 1, file);
        }
        int filler_chars = 28 - strlen(filename);
        unsigned empty = 0;
        for (int byte = 0; byte < filler_chars; byte++) {
            fwrite(&empty, 1, 1, file);
        }

        // Create index block
        fsetpos(file, &position);
        fseek(file, 2048 * free_block, SEEK_CUR);
        for (int byte = 0; byte < 5; byte++) {
            fwrite(&empty, 1, 1, file);
        }

        // Generate osFile
        osFile* stream = malloc(sizeof(osFile));
        stream->mode = mode;
        stream->partition_pos = partition_info[0];
        stream->partition_size = partition_info[1];
        stream->index_block = free_block;
        stream->bytes = 0;

        // Free memory
        free(partition_info);
        free(buffer);
        fclose(file);

        // Return osFile
        return stream;

    } else if (file_found) {
        OS_ERROR = FileExists;

    } else if (new_entry < 0) {
        OS_ERROR = NoDirectoryEntry;
    }

    free(partition_info);
    free(buffer);
    fclose(file);
    return NULL;
}

// Read file
int os_read(osFile* file_desc, void* buffer, int nbytes) {

    // Check osFile with correct mode
    if (file_desc->mode != 'r') {
        OS_ERROR = InvalidFileMode;
        return 0;
    }

    // Check positive nbytes
    if (nbytes < 0) {
        OS_ERROR = InvalidBytesNumber;
        return 0;
    }

    // Move to index block for file
    FILE* file = fopen(disk_path, "rb");
    unsigned long* file_size = calloc(1, sizeof(unsigned long));
    unsigned* data_block = calloc(1, sizeof(unsigned));

    // 1024 skips MBT, partition_pos * 2048 skips to partition, index_block skips to index block for file
    fseek(file, 1024 + (file_desc->partition_pos + file_desc->index_block) * 2048, SEEK_SET);
    fpos_t index_position; // Holds position for index block
    fgetpos(file, &index_position);
    bool reading = true;

    // Read current file size
    fsetpos(file, &index_position);
    fread(file_size, 5, 1, file);
    *file_size = to_big_endian_long(*file_size, 5);
    unsigned bytes_read = 0;

    // Read loop
    while (reading) {

        // Find data block location
        unsigned current_block = floor((double) file_desc->bytes / 2048);
        fsetpos(file, &index_position);
        fseek(file, 5 + 3 * current_block, SEEK_CUR);
        fread(data_block, 3, 1, file);
        *data_block = to_big_endian(*data_block, 3);
        fseek(file, 1024 + (file_desc->partition_pos + *data_block) * 2048, SEEK_SET);
        unsigned offset = file_desc->bytes % 2048;
        fseek(file, offset, SEEK_CUR);

        // Read data block
        unsigned remaining_data = *file_size - file_desc->bytes;
        unsigned free_space = 2048 - offset;
        unsigned remaining = nbytes - bytes_read;
        unsigned bytes_to_read = free_space;
        if ((remaining < free_space) && (remaining <= remaining_data)) {
            bytes_to_read = remaining;
        } else if ((remaining_data < free_space) && (remaining_data <= remaining)) {
            bytes_to_read = remaining_data;
        }
        char* origin = buffer;
        fread(origin + bytes_read, 1, bytes_to_read, file);
        file_desc->bytes += bytes_to_read;
        bytes_read += bytes_to_read;

        // Check completion
        if ((bytes_read == nbytes) || (file_desc->bytes == *file_size)) {
            reading = false;
        }
    }

    // Memory cleaning
    free(file_size);
    free(data_block);
    fclose(file);

    if (bytes_read < nbytes) {
        OS_ERROR = BytesExceeded;
    }

    return bytes_read;
}

// Write file
int os_write(osFile* file_desc, void* buffer, int nbytes) {

    // osFile is in read mode
    if (file_desc->mode != 'w') {
        OS_ERROR = InvalidFileMode;
        return 0;
    }

    // Check positive nbytes
    if (nbytes < 0) {
        OS_ERROR = InvalidBytesNumber;
        return 0;
    }

    // Move to index block for file
    FILE* file = fopen(disk_path, "rb+");
    unsigned long* file_size = calloc(1, sizeof(unsigned long));
    unsigned* data_block = calloc(1, sizeof(unsigned));

    // 1024 skips MBT, partition_pos * 2048 skips to partition, index_block skips to index block for file
    fseek(file, 1024 + (file_desc->partition_pos + file_desc->index_block) * 2048, SEEK_SET);
    fpos_t index_position; // Holds position for index block
    fgetpos(file, &index_position);
    file_desc->bytes = 0;
    bool writing = true;

    // Write loop
    while (writing) {

        // Read current file size
        fsetpos(file, &index_position);
        fread(file_size, 5, 1, file);
        *file_size = to_big_endian_long(*file_size, 5);

        // Allocate new data block
        if (!(*file_size % 2048)) {

            // Get block count
            unsigned block_count = *file_size / 2048;
            if (block_count == 681) { // Max file size reached
                OS_ERROR = MaxFileSizeReached;
                free(file_size);
                free(data_block);
                fclose(file);
                return file_desc->bytes;
            }

            // Get new data block
            unsigned new_block = get_free_block(file_desc->partition_pos, file_desc->partition_size, file);
            if (!new_block) {
                free(file_size);
                free(data_block);
                fclose(file);
                return file_desc->bytes;
            }

            // Create pointer to new data block
            fsetpos(file, &index_position);
            fseek(file, 5 + 3 * block_count, SEEK_CUR);
            new_block = to_big_endian(new_block, 3);
            fwrite(&new_block, 3, 1, file);
        }

        // Find data block location
        unsigned current_block = floor((double) *file_size / 2048);
        fsetpos(file, &index_position);
        fseek(file, 5 + 3 * current_block, SEEK_CUR);
        fread(data_block, 3, 1, file);
        *data_block = to_big_endian(*data_block, 3);
        fseek(file, 1024 + (file_desc->partition_pos + *data_block) * 2048, SEEK_SET);
        unsigned offset = *file_size % 2048;
        fseek(file, offset, SEEK_CUR);

        // Write to data block
        unsigned free_space = 2048 - offset;
        unsigned remaining = nbytes - file_desc->bytes;
        unsigned bytes_to_write = free_space;
        if (remaining < free_space) {
            bytes_to_write = remaining;
        }
        char* origin = buffer;
        fwrite(origin + file_desc->bytes, 1, bytes_to_write, file);
        file_desc->bytes += bytes_to_write;

        // Update file size
        *file_size += bytes_to_write;
        *file_size = to_big_endian_long(*file_size, 5);
        fsetpos(file, &index_position);
        fwrite(file_size, 5, 1, file);

        // Check completion
        if (file_desc->bytes == nbytes) {
            writing = false;
        }
    }

    // Memory cleaning
    free(file_size);
    free(data_block);
    fclose(file);

    if (file_desc->bytes < nbytes) {
        OS_ERROR = BytesExceeded;
    }

    return file_desc->bytes;
}

// Close file
int os_close(osFile* file_desc) {
    free(file_desc);
    return 0;
}

// Remove file
int os_rm(char* filename) {

    // Get partition
    unsigned* partition_info = find_partition();
    FILE* file = fopen(disk_path, "rb+");
    unsigned* buffer = calloc(1, sizeof(unsigned));
    char* name_buffer = calloc(28, sizeof(char));

    // 1024 skips MBT, partition_start * 2048 skips to partition
    fseek(file, 1024 + partition_info[0] * 2048, SEEK_SET);
    fpos_t position; // Holds position for partition directory
    fgetpos(file, &position);

    // Search for file with the same name
    bool file_found = false;
    unsigned index_pos = 0;
    for (int entry = 0; entry < 64; entry++) {
        fsetpos(file, &position);
        fseek(file, entry * 32, SEEK_CUR);
        fread(buffer, 1, 1, file);
        if (*buffer) {
            fread(buffer, 3, 1, file);
            index_pos = to_big_endian(*buffer, 3);
            fread(name_buffer, 1, 28, file);
            file_found = !strcmp(name_buffer, filename);
            if (file_found) {
                fsetpos(file, &position);
                fseek(file, entry * 32, SEEK_CUR);
                unsigned empty = 0;
                fwrite(&empty, 1, 1, file);
                break;
            }
        }
    }

    if (file_found) {
        fsetpos(file, &position);
        fseek(file, 2048, SEEK_CUR);
        fgetpos(file, &position);  // Save position after directory

        // Free index block bit
        int bitmap = floor((double) index_pos / 16384);
        int byte_pos = floor((double) (index_pos % 16384) / 8);
        fseek(file, 2048 * bitmap + byte_pos, SEEK_CUR);
        fread(buffer, 1, 1, file);
        unsigned new_byte = (128 >> (index_pos % 8)) ^ *buffer;
        fsetpos(file, &position);
        fseek(file, 2048 * bitmap + byte_pos, SEEK_CUR);
        fwrite(&new_byte, 1, 1, file);

        // Free data block bits
        fsetpos(file, &position);
        fseek(file, 2048 * (index_pos - 1), SEEK_CUR);
        unsigned long* file_size = calloc(1, sizeof(unsigned long));
        unsigned* p_buffer = calloc(1, sizeof(unsigned));
        fread(file_size, 5, 1, file);
        *file_size = to_big_endian_long(*file_size, 5);
        int n_pointers = ceil((double) *file_size / 2048);
        for (int p = 0; p < n_pointers; p++) {
            fsetpos(file, &position);
            fseek(file, 2048 * (index_pos - 1) + 5 + 3 * p, SEEK_CUR);
            fread(p_buffer, 3, 1, file);  // Current pointer to data
            *p_buffer = to_big_endian(*p_buffer, 3);
            bitmap = floor((double) *p_buffer / 16384);
            byte_pos = floor((double) (*p_buffer % 16384) / 8);
            fsetpos(file, &position);
            fseek(file, 2048 * bitmap + byte_pos, SEEK_CUR);
            fread(buffer, 1, 1, file);
            new_byte = (128 >> (*p_buffer % 8)) ^ *buffer;
            fsetpos(file, &position);
            fseek(file, 2048 * bitmap + byte_pos, SEEK_CUR);
            fwrite(&new_byte, 1, 1, file);
        }

        // Memory cleaning
        free(partition_info);
        free(name_buffer);
        free(file_size);
        free(p_buffer);
        free(buffer);
        fclose(file);

        // Success Message
        fprintf(stdout, GRN "\n>>> File successfully deleted!\n");
        fprintf(stdout, DEFAULT "");

        return 0;

    }

    // Set error for non existent file
    OS_ERROR = FileNotFound;

    // Memory cleaning
    free(partition_info);
    free(name_buffer);
    free(buffer);
    fclose(file);
    return 1;
}

// ----------- Utils ----------- //

// Find location & size for current partition
unsigned* find_partition() {

    // Assign memory buffers
    unsigned* partition_info = malloc(2 * sizeof(unsigned));
    unsigned* partition_header = calloc(1, sizeof(unsigned));
    unsigned* abs_id = calloc(1, sizeof(unsigned));
    unsigned* partition_size = calloc(1, sizeof(unsigned));

    // Try to open disk_path file
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
            partition_info[0] = to_big_endian(*abs_id, 3);

            // Get size for partition
            fread(partition_size, 4, 1, file);
            partition_info[1] = to_big_endian(*partition_size, 4);

            // Free memory and return start pointer
            free(partition_header);
            free(abs_id);
            free(partition_size);
            fclose(file);
            return partition_info;
        }
    }

    // Memory cleaning
    free(partition_header);
    free(abs_id);
    free(partition_size);
    free(partition_info);
    fclose(file);

    // Partition not found
    OS_ERROR = PartitionNotFound;
    return NULL;
}

// Display free & used blocks
int* count_bitmap_blocks(char* bitmap, int limit) {
    int used_blocks = 0;
    int free_blocks = 0;
    int max_bytes = ceil((double) limit / 8);
    for (int byte = 0; byte < max_bytes; byte++) {
        char current = bitmap[byte];
        int bit_limit = 8;
        if ((byte == max_bytes - 1) && (limit % 8)) {
            bit_limit = limit % 8;
        }
        for (int bit = 0; bit < bit_limit; bit++) {
            unsigned is_used = current & 1;
            if (is_used) {
                used_blocks++;
            }
            else {
                free_blocks++;
            }
            current >>= 1;
        }
    }
    int* block_data = calloc(2, sizeof(int));
    block_data[0] = used_blocks;
    block_data[1] = free_blocks;
    return block_data;
}

// Sort valid partitions according to order in disk
void sort_mbt(unsigned* partitions, unsigned* sizes, int n) {
    for (int idx = 0; idx < n; idx++) {
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

// Converts unsigned between little endian & big endian
unsigned to_big_endian(unsigned n, int n_bytes) {
    unsigned first = n << 24;
    unsigned second = ((n >> 8) & 0xFF) << 16;
    unsigned third = ((n >> 16) & 0xFF) << 8;
    unsigned fourth = n >> 24;
    unsigned big_endian = first | second | third | fourth;
    return big_endian >> 8 * (4 - n_bytes);
}

// Converts unsigned long between little endian & big endian
unsigned long to_big_endian_long(unsigned long n, int n_bytes) {
    unsigned long first = n << 56;
    unsigned long second = ((n >> 8) & 0xFF) << 48;
    unsigned long third = ((n >> 16) & 0xFF) << 40;
    unsigned long fourth = ((n >> 24) & 0xFF) << 32;
    unsigned long fifth = ((n >> 32) & 0xFF) << 24;
    unsigned long sixth = ((n >> 40) & 0xFF) << 16;
    unsigned long seventh = ((n >> 48) & 0xFF) << 8;
    unsigned long eight = n >> 56;
    unsigned long big_endian = first | second | third | fourth | fifth | sixth | seventh | eight;
    return big_endian >> 8 * (8 - n_bytes);
}

// Reserve new block for data
unsigned get_free_block(unsigned partition_start, unsigned partition_size, FILE* file) {
    unsigned* buffer = calloc(1, sizeof(unsigned));

    // Move to partition
    fseek(file, 1024 + (partition_start + 1) * 2048, SEEK_SET);
    fpos_t bitmap_pos; // Holds position for partition bitmaps
    fgetpos(file, &bitmap_pos);
    int bitmap_count = ceil((double) partition_size / 16384);

    // Search for free block
    unsigned free_block = 0;
    for (int bitmap = 0; bitmap < bitmap_count; bitmap++) {
        for (int byte = 0; byte < 2048; byte++) {
            fread(buffer, 1, 1, file);
            unsigned current_byte = *buffer;
            for (int bit = 0; bit < 8; bit++) {
                if (!((current_byte >> (7 - bit)) & 1)) {  // Update bitmap for index block
                    free_block = bitmap * 16384 + byte * 8 + bit;
                    fsetpos(file, &bitmap_pos);
                    fseek(file, bitmap * 2048 + byte, SEEK_CUR);
                    unsigned new_byte = (128 >> bit) | current_byte;
                    fwrite(&new_byte, 1, 1, file);
                    break;
                }
            }
            if (free_block) {
                break;
            }
        }
        if (free_block) {
            break;
        }
    }

    // Check for available free blocks (free_blocks > 0)
    if (!free_block) {
        OS_ERROR = NoBlocksAvailable;
    }

    // Memory cleaning
    free(buffer);
    return free_block;
}

// Get global variables
char* get_diskname() {
    return disk_path;
}

// Free memory for global variables
void os_unmount() {
    if (disk_path != NULL) {
        free(disk_path);
        return;
    }
    OS_ERROR = DiskNotFound;
}

// Bonus
void os_strerror(Error err) {
    switch (err) {
    case DiskNotFound:
        fprintf(stderr, RED "\nIOError: Disk Not Found!\n");
        break;
    case NoSpaceAvailable:
        fprintf(stderr, RED "\nIOError: No space available in disk!\n");
        break;
    case NoBlocksAvailable:
        fprintf(stderr, RED "\nIOError: No blocks available in partition!\n");
        break;
    case NoDirectoryEntry:
        fprintf(stderr, RED "\nIOError: No entries available in partition directory!\n");
        break;
    case PartitionNotFound:
        fprintf(stderr, RED "\nInvalidPartitionError: Partition Not Found!\n");
        break;
    case PartitionExists:
        fprintf(stderr, RED "\nInvalidPartitionError: Partition already exists!\n");
        break;
    case PartitionOutOfRange:
        fprintf(stderr, RED "\nIndexError: Partition ID/Size Out of Range!\n");
        break;
    case InvalidBitmapIndex:
        fprintf(stderr, RED "\nIndexError: Invalid Bitmap Index!\n");
        break;
    case InvalidFileName:
        fprintf(stderr, RED "\nInvalidInputError: Filename Too Long!\n");
        break;
    case InvalidBytesNumber:
        fprintf(stderr, RED "\nInvalidInputError: Invalid number of bytes to read/write!\n");
        break;
    case InvalidFileMode:
        fprintf(stderr, RED "\nFileIOError: Action is not allowed by file mode!\n");
        break;
    case FileNotFound:
        fprintf(stderr, RED "\nFileIOError: File not found!\n");
        break;
    case FileExists:
        fprintf(stderr, RED "\nFileIOError: File already exists!\n");
        break;
    case BytesExceeded:
        fprintf(stderr, YEL "\nWARNING: Effective number of bytes read/written is different from nbytes!\n");
        break;
    case MaxFileSizeReached:
        fprintf(stderr, YEL "\nWARNING: Max file size reached during operation!\n");
        break;
    default:
        break;
    }

    // Reset
    fprintf(stderr, DEFAULT "");
    OS_ERROR = NoError;
}
