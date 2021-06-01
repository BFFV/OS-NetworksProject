#include "os_API.h"

// Declare functions
void manage_partitions();
void manage_this();
void manage_files();
int download_file(char* filename);
int upload_file(char* filename, char* disk_filename);

// Manage partitions
void manage_partitions() {

    // List all available partitions
    os_mbt();

    // Ask for command
    printf("\nSelect one of the following commands:\n\n");
    printf(">>> [mount]     Mount a partition inside the disk\n");
    printf(">>> [create]    Create a new partition inside the disk\n");
    printf(">>> [delete]    Delete a partition from the disk\n");
    printf(">>> [reset]     Delete all partitions from the disk\n");
    printf(">>> [exit]      Exit program\n");

    // Read command
    char command[12];
    printf("\n> Command: ");
    scanf("%s", command);

    // Create new partition
    if (!strcmp(command, "create")) {
        printf("\n>>> Enter an ID [int] and size [int] for the new partition...\n\n");
        int partition_id;
        int size;
        printf("> ID: ");
        scanf("%d", &partition_id);
        printf("> Size: ");
        scanf("%d", &size);
        os_create_partition(partition_id, size);
        manage_partitions();

    } else if (!strcmp(command, "delete")) { // Delete partition
        printf("\n>>> Enter the ID [int] of the partition to delete...\n\n");
        int partition_id;
        printf("> ID: ");
        scanf("%d", &partition_id);
        os_delete_partition(partition_id);
        manage_partitions();

    } else if (!strcmp(command, "reset")) { // Reset disk partitions
        printf("\n>>> This will delete all data from the disk, are you sure? (y/n)\n");
        char answer[1];
        scanf("%s", answer);
        if (!strcmp(answer, "y")) {
            reset_mbt();
        }
        manage_partitions();

    } else if (!strcmp(command, "mount")) { // Mount partition
        printf("\n>>> Enter the ID [int] of the partition to mount...\n\n");
        int partition_id;
        printf("> ID: ");
        scanf("%d", &partition_id);
        char disk_path[strlen(get_diskname()) + 1];
        strcpy(disk_path, get_diskname());
        os_mount(disk_path, partition_id);
        manage_this();

    } else if (!strcmp(command, "exit")) { // Exit
        // Unmount disk & exit
        os_unmount();
        exit(0);
    } else { // Invalid
        manage_partitions();
    }
}

// Manage specific partition
void manage_this() {

    // Ask for command
    printf("\nSelect one of the following commands:\n\n");
    printf(">>> [files]   Manage files in current partition\n");
    printf(">>> [bitmap]  Displays current partition BitMap\n");
    printf(">>> [ls]      Display all files in current partition\n");
    printf(">>> [back]    Go back\n");

    // Read command
    char command[12];
    printf("\n> Command: ");
    scanf("%s", command);

    // List files in partition
    if (!strcmp(command, "ls")) {
        os_ls();
        manage_this();

    } else if (!strcmp(command, "bitmap")) { // Display partition bitmap
        printf("\n>>> Enter the BitMap number [int] (0 for ALL)...\n\n");
        printf("> Bitmap: ");
        int bitmap;
        scanf("%d", &bitmap);
        os_bitmap(bitmap);
        manage_this();

    } else if (!strcmp(command, "files")) { // Manage files
        manage_files();

    } else if (!strcmp(command, "back")) { // Go back
        manage_partitions();

    } else { // Invalid
        manage_this();
    }
}

// Manage files inside partition
void manage_files() {

    // Ask for command
    printf("\nSelect one of the following commands:\n\n");
    printf(">>> [create]    Create new text file\n");
    printf(">>> [download]  Download file\n");
    printf(">>> [upload]    Upload file\n");
    printf(">>> [exists]    Check if file exists\n");
    printf(">>> [rm]        Remove file\n");
    printf(">>> [ls]        Display all files in current partition\n");
    printf(">>> [back]      Go back\n");

    // Read command
    char command[12];
    printf("\n> Command: ");
    scanf("%s", command);

    // Create new text file
    if (!strcmp(command, "create")) {
        printf("\n>>> Enter the filename and text separated by a space...\n\n");
        char filename[256];
        char text[256];
        printf("> File & Text: ");
        scanf("%s %[^\n]s", filename, text);
        osFile* file = os_open(filename, 'w');
        os_write(file, text, strlen(text));
        os_close(file);
        printf("\n>>> File created and data entered successfully!\n");
        manage_files();

    } else if (!strcmp(command, "download")) { // Download file
        printf("\n>>> Enter the filename...\n\n");
        char filename[256];
        printf("> Filename: ");
        scanf("%s", filename);
        int error = download_file(filename);
        if (!error) {
            printf("\n>>> File downloaded successfully!\n");
        }
        manage_files();

    } else if (!strcmp(command, "upload")) { // Upload file
        printf("\n>>> Enter the path for the file you want to upload...\n\n");
        char pc_filename[256];
        printf("> Path: ");
        scanf("%s", pc_filename);
        printf("\n>>> Enter the filename you want to save it as...\n\n");
        char filename[256];
        printf("> Filename: ");
        scanf("%s", filename);
        int error = upload_file(pc_filename, filename);
        if (!error) {
            printf("\n>>> File uploaded successfully!!!\n");
        }
        manage_files();

    } else if (!strcmp(command, "exists")) { // Check if file exists
        printf("\n>>> Enter the name for the file you want to check...\n\n");
        char pc_filename[256];
        printf("> Filename: ");
        scanf("%s", pc_filename);
        bool exists = os_exists(pc_filename);
        if (exists) {
            printf("\nFile exists!\n");
        } else {
            printf("\nFile doesn't exist!\n");
        }
        manage_files();

    } else if (!strcmp(command, "rm")) { // Remove file
        printf("\n>>> Enter the name of the file you want to remove...\n\n");
        char filename[28];
        printf("> Filename: ");
        scanf("%s", filename);
        os_rm(filename);
        manage_files();

    } else if (!strcmp(command, "ls")) { // List files
        os_ls();
        manage_files();

    } else if (!strcmp(command, "back")) { // Go back
        manage_this();

    } else { // Invalid
        manage_files();
    }
}

// Upload file to disk
int upload_file(char* filename, char* disk_filename) {
    FILE* input = fopen(filename, "rb");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(disk_filename, 'w');
    if (file != NULL) {
        int bytes = 1;
        size_t bytes_read = 0;
        bool uploading = true;
        while (uploading) {
            bytes_read = fread(buffer, 1, 4096, input);
            bytes = os_write(file, buffer, (int) bytes_read);
            if ((bytes_read < 4096) || (!bytes)) {
                uploading = false;
            }
        }
    }
    os_close(file);
    free(buffer);
    fclose(input);
    if (file != NULL) {
        return 0;
    }
    return 1;
}

// Download file from disk
int download_file(char* filename) {
    FILE* output = fopen(filename, "w");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(filename, 'r');
    if (file != NULL) {
        int bytes = 1;
        while (bytes) {
            bytes = os_read(file, buffer, 4096);
            fwrite(buffer, 1, bytes, output);
        }
    }
    os_close(file);
    free(buffer);
    fclose(output);
    if (file != NULL) {
        return 0;
    }
    return 1;
}

// OldSchool File System Demo
int main(int argc, char** argv) {

    // Check input
    if (argc < 2) {
        printf("[Error] Not enough arguments were provided!\n");
        return 1;
    }

    // Mount disk
    printf("Loading Disk...\n");
    os_mount(argv[1], 0);

    // First: All partitions
    manage_partitions();

    // Second: This partition [bitmap, os ls, os exists]
    manage_this();

    return 0;
}
