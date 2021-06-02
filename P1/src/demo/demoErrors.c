#include "os_API.h"

// Declare functions
void request_test();
int download_file(char* filename);
int upload_file(char* filename, char* disk_filename);

// Manage tests
void request_test() {

    // Ask for command
    printf("\nSelect one of the following tests:\n\n");
    printf(">>> [fullDisk]          No space available for a new partition\n");
    printf(">>> [fillPartition]     Mounted partition runs out of space\n");
    printf(">>> [fileIO]            Try to read/write a file opened in the wrong mode\n");
    printf(">>> [ls]                Display all files in current partition\n");
    printf(">>> [create]            Create a new partition inside the disk\n");
    printf(">>> [delete]            Delete a partition from the disk\n");
    printf(">>> [mount]             Mount a partition inside the disk\n");
    printf(">>> [reset]             Delete all partitions from the disk\n");
    printf(">>> [exit]              Exit program\n");

    // Read command
    char command[12];
    printf("\n> Command: ");
    scanf("%s", command);

    // Create partitions ...
    if (!strcmp(command, "fullDisk")) {
        printf("\n>>> Creating partitions of size %d blocks...\n\n", 131072);
        int partition_id = 1;
        while (true) {
            os_create_partition(partition_id, 131072);
            if (OS_ERROR != NoError) {
                os_strerror(OS_ERROR);
                break;
            }
            fprintf(stdout, GRN " >#: Partition %d created without errors!\n", partition_id);
            partition_id++;
        }
        request_test();
    }

    // Fill a partition with one file
    if (!strcmp(command, "fillPartition")) {
        printf("\n>>> Enter the path to the file you want to fill the disk with...\n\n");
        char pc_filename[256];
        printf("> Path: ");
        scanf("%s", pc_filename);
        int file_id = 1;
        char* filename = "fillTest";
        char* this_filename;
        char str_id[2];
        while (true) {
            sprintf(str_id, "%d", file_id);
            this_filename = strcat(filename, str_id);
            if (upload_file(pc_filename, this_filename)) {
                break;
            }
            fprintf(stdout, GRN " >#: File %d created without errors!", file_id);
            fprintf(stdout, DEFAULT "");
            file_id++;
        }
        request_test();
    }

    // Test
    if (!strcmp(command, "fileIO")) {
        char filename[256];
        printf("\n>>> Enter the filename...\n\n");
        printf("> Filename: ");
        scanf("%c", filename);
        char mode[1];
        printf("\n>>> Enter the file mode [w/r] you want to try to execute ...\n\n");
        printf("> Mode: ");
        scanf("%c", mode);
        int nbytes;
        printf("\n>>> Enter number of bytes to write/read...\n\n");
        printf("> nBytes: ");
        scanf("%d", &nbytes);

        // Open and try to write/read file
        osFile* file = os_open(filename, mode);

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
        request_test();

    if (!strcmp(command, "ls")) { // List files
        os_ls();
        request_test();
    }

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
        request_test();

    } else if (!strcmp(command, "delete")) { // Delete partition
        printf("\n>>> Enter the ID [int] of the partition to delete...\n\n");
        int partition_id;
        printf("> ID: ");
        scanf("%d", &partition_id);
        os_delete_partition(partition_id);
        if (OS_ERROR != NoError) {
            os_strerror(OS_ERROR);
        }
        request_test();

    } else if (!strcmp(command, "reset")) { // Reset disk partitions
        printf("\n>>> This will delete all data from the disk, are you sure? (y/n)\n");
        char answer[1];
        scanf("%s", answer);
        if (!strcmp(answer, "y")) {
            reset_mbt();
        }
        request_test();

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
        }
        request_test();

    } else if (!strcmp(command, "exit")) { // Unmount disk & exit
        os_unmount();
        exit(0);

    } else { // Invalid
        request_test();
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

// OldSchool File System Demo for Errors
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

    // Tests
    request_test();

    return 0;
}
