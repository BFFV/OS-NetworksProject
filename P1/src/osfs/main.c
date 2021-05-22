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
    os_mbt();
    for (int p = 0; p < 128; p++) {
        os_create_partition(p, 16384);
        os_mount(argv[1], p);
    }
    os_bitmap(0);
    os_mbt();
    clean_vars();
    return 0;
}
