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
    os_mbt();
    osFile* bitwiser = os_open("bitwiser.bin", 'w');
    os_close(bitwiser);
    os_ls();
    os_bitmap(0);
    clean_vars();
    return 0;
}
