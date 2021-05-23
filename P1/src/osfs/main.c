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
    os_create_partition(0, 16384);
    os_bitmap(0);
    osFile* bitwiser = os_open("bitwiser.bin", 'w');
    os_close(bitwiser);
    bitwiser = os_open("no_disks_uploaded_yet.xd", 'w');
    os_close(bitwiser);
    bitwiser = os_open("ggs.txt", 'w');
    os_close(bitwiser);
    bitwiser = os_open("fffff.lol", 'w');
    os_close(bitwiser);
    os_bitmap(0);
    os_ls();
    os_exists("bitwiser.bin");
    os_rm("bitwiser.bin");
    os_rm("ggs.txt");
    os_ls();
    os_exists("bitwiser.bin");
    bitwiser = os_open("test.passed", 'w');
    os_create_partition(1, 16384 * 4);
    os_close(bitwiser);
    os_ls();
    os_mbt();
    os_delete_partition(1);
    os_mbt();
    clean_vars();
    return 0;
}
