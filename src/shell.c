#include "../include/sfs.h"
// gcc shell.c -o FS `pkg-config fuse --cflags --libs`


//MAIN FS
// ./shell -f /home/alexander/mnt
// fusermount -u /home/alexander/mnt


//SDCARD
// sudo mount /dev/mmcblk0p1 /home/alexander/sdCard/
// sudo ./shell -f -o allow_other /home/alexander/sdCard/mnt/
// sudo fusermount -u /home/alexander/sdCard/mnt/


int main(int argc, char *argv[]) {
    printf("NEW SYSTEM\n");
    superblock_init();
    root_dir_init();

    return fuse_main(argc, argv, &operations, NULL);
}