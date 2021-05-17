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
    os_bitmap((unsigned) 1);
    clean_vars();
    return 0;
}
