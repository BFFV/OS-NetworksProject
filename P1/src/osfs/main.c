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
    printf(">>> [fill]      Fill disk with partitions\n");
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
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        manage_partitions();

    } else if (!strcmp(command, "delete")) { // Delete partition
        printf("\n>>> Enter the ID [int] of the partition to delete...\n\n");
        int partition_id;
        printf("> ID: ");
        scanf("%d", &partition_id);
        os_delete_partition(partition_id);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
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
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
            manage_partitions();
        } else {
            manage_this();
        }

    } else if (!strcmp(command, "exit")) { // Unmount disk & exit
        os_unmount();
        exit(0);

    } else if (!strcmp(command, "fill")) { // Fill disk for testing
        printf("\n>>> Enter the Size [int] of each partition...\n\n");
        int size;
        printf("> Size: ");
        scanf("%d", &size);
        printf("\n>>> Creating partitions of size %d blocks...\n\n", size);
        int partition_id = 0;
        while (true) {
            os_create_partition(partition_id, size);
            if (OS_ERROR != NoError) {
                os_strerror(OS_ERROR);
                break;
            }
            fprintf(stdout, GRN " >#: Partition %d created without errors!\n", partition_id);
            partition_id++;
        }
        manage_partitions();

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
    printf(">>> [fill]    Fill partition with files\n");
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
        printf("\n>>> Enter the Bitmap number [int] (0 for ALL)...\n\n");
        printf("> Bitmap: ");
        int bitmap;
        scanf("%d", &bitmap);
        os_bitmap(bitmap);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        manage_this();

    } else if (!strcmp(command, "files")) { // Manage files
        manage_files();

    } else if (!strcmp(command, "back")) { // Go back
        manage_partitions();

    } else if (!strcmp(command, "fill")) { // Fill partition for testing
        printf("\n>>> Enter the path to the file you want to fill the disk with...\n\n");
        char pc_filename[256];
        printf("> Path: ");
        scanf("%s", pc_filename);
        int file_id = 1;
        char str_id[2];
        while (true) {
            char filename[28] = "fill";
            sprintf(str_id, "%d", file_id);
            strcat(filename, str_id);
            if (upload_file(pc_filename, filename)) {
                break;
            }
            fprintf(stdout, GRN " >#: File %d created without errors!\n", file_id);
            fprintf(stdout, DEFAULT "");
            file_id++;
        }
        manage_this();

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
    printf(">>> [fileTest]  Try to write to a file opened in the wrong mode\n");
    printf(">>> [back]      Go back\n");

    // Read command
    char command[12];
    printf("\n> Command: ");
    scanf("%s", command);

    // Create new text file
    if (!strcmp(command, "create")) {
        printf("\n>>> Enter the filename and content (text) separated by a space...\n\n");
        char filename[256];
        char text[256];
        printf("> File & Text: ");
        scanf("%s %[^\n]s", filename, text);
        osFile* file = os_open(filename, 'w');
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        } else {
            os_write(file, text, strlen(text));
            if (OS_ERROR != NoError) {
                os_strerror(OS_ERROR);
            }
            os_close(file);
            fprintf(stdout, GRN "\n>>> File created and data entered successfully!\n");
            fprintf(stdout, DEFAULT "");
        }
        manage_files();

    } else if (!strcmp(command, "download")) { // Download file
        printf("\n>>> Enter the filename...\n\n");
        char filename[256];
        printf("> Filename: ");
        scanf("%s", filename);
        int error = download_file(filename);
        if (!error) {
            fprintf(stdout, GRN "\n>>> File downloaded successfully!\n");
            fprintf(stdout, DEFAULT "");
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
            fprintf(stdout, GRN "\n>>> File uploaded successfully!!!\n");
            fprintf(stdout, DEFAULT "");
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
        } else if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        } else {
            printf("\nFile doesn't exist!\n");
        }
        manage_files();

    } else if (!strcmp(command, "rm")) { // Remove file
        printf("\n>>> Enter the name of the file you want to remove...\n\n");
        char filename[256];
        printf("> Filename: ");
        scanf("%s", filename);
        os_rm(filename);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        manage_files();

    } else if (!strcmp(command, "ls")) { // List files
        os_ls();
        manage_files();

    } else if (!strcmp(command, "back")) { // Go back
        manage_this();

    } else if (!strcmp(command, "fileTest")) {
        char filename[256];
        printf("\n>>> Enter the filename...\n\n");
        printf("> Filename: ");
        scanf("%s", filename);
        int nbytes;
        printf("\n>>> Enter number of bytes to write...\n\n");
        printf("> nBytes: ");
        scanf("%d", &nbytes);

        // Open and try to write/read file
        osFile* file = os_open(filename, 'w');

        // Test write with selected mode
        char buffer[256];

        // Test read with selected mode
        printf("\n>>> Trying to read from file...\n\n");
        os_read(file, buffer, nbytes);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }

        printf("\n>>> Trying to write on file...\n\n");
        os_write(file, buffer, nbytes);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        os_close(file);
        manage_files();

    } else { // Invalid
        manage_files();
    }
}

// Upload file to disk
int upload_file(char* filename, char* disk_filename) {
    FILE* input = fopen(filename, "rb");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(disk_filename, 'w');
    if (OS_ERROR != NoError) {
        os_strerror(OS_ERROR);
        free(buffer);
        fclose(input);
        return 1;
    }
    int bytes = 1;
    size_t bytes_read = 0;
    bool uploading = true;
    while (uploading) {
        bytes_read = fread(buffer, 1, 4096, input);
        bytes = os_write(file, buffer, (int) bytes_read);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        if ((bytes_read < 4096) || (!bytes)) {
            uploading = false;
        }
    }
    os_close(file);
    free(buffer);
    fclose(input);
    return 0;
}

// Download file from disk
int download_file(char* filename) {
    FILE* output = fopen(filename, "w");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(filename, 'r');
    if (OS_ERROR != NoError) {
        os_strerror(OS_ERROR);
        free(buffer);
        fclose(output);
        return 1;
    }
    int bytes = 1;
    while (bytes) {
        bytes = os_read(file, buffer, 4096);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        fwrite(buffer, 1, bytes, output);
    }
    os_close(file);
    free(buffer);
    fclose(output);
    return 0;
}

// OldSchool File System Demo
int main(int argc, char** argv) {

    // Check input
    if (argc < 2) {
        printf("[Error] Not enough arguments were provided!\n");
        return 1;
    }

    // Mount disk initially
    printf("\nLoading Disk...\n");
    os_mount(argv[1], -1);
    if (OS_ERROR != NoError) {
        os_strerror(OS_ERROR);
        return 1;
    }

    // First: All partitions
    manage_partitions();

    // Second: This partition [bitmap, os ls, os exists]
    manage_this();

    return 0;
}
