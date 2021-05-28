#include "os_API.h"


// OldSchool File System Demo
int main(int argc, char** argv) {

    // Check input
    if (argc < 2) {
        printf("[Error] Not enough arguments were provided!\n");
        return 1;
    }

    // Demo
    os_mount(argv[1], 0);
    reset_mbt();
    os_create_partition(0, 16384);
    char* buffer = malloc(500000 * sizeof(char));
    char* receiver = malloc(500000 * sizeof(char));
    osFile* bitwiser = os_open("bitwiser.bin", 'w');
    os_write(bitwiser, buffer, 500000);
    os_close(bitwiser);
    bitwiser = os_open("bitwiser.txt", 'w');
    os_write(bitwiser, buffer, 500000);
    os_close(bitwiser);
    os_bitmap(0);
    os_rm("bitwiser.bin");
    os_bitmap(0);
    bitwiser = os_open("bitwiser.bin", 'w');
    os_write(bitwiser, buffer, 500000);
    os_close(bitwiser);
    os_bitmap(0);
    os_unmount();
    return 0;
}
