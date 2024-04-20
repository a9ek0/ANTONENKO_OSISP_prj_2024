#include "../include/sfs.h"
// gcc shell.c -o FS `pkg-config fuse --cflags --libs`


//MAIN FS
// ./shell -f /home/alexander/mnt
// fusermount -u /home/alexander/mnt


//SDCARD
// sudo mount /dev/mmcblk0p1 /home/alexander/sdCard/mnt
// sudo ./shell -f -o allow_other /home/alexander/sdCard/mnt/
// sudo fusermount -u /home/alexander/sdCard/mnt/


int main(int argc, char *argv[]) {
/*
    FILE *fd = fopen("file_structure.bin", "rb");
    if (fd) {
        printf("SYSTEM RESTORED\n");
        if (fread(&file_array, sizeof(filetype) * 31, 1, fd) != 1) {
            // Handle read error or incomplete read
            fprintf(stderr, "Error reading file structure.\n");
        }
        fclose(fd); // Ensure the file is closed

        int child_startindex = 1;
        file_array[0].parent = NULL;

        for (int i = 0; i < 6; i++) {
            file_array[i].num_children = 0;
            file_array[i].children = NULL;
            for (int j = child_startindex; j < child_startindex + 5; j++) {
                if (file_array[j].valid) {
                    add_child(&file_array[i], &file_array[j]);
                }
            }
            child_startindex += 5;
        }

        root = &file_array[0];

        FILE *fd1 = fopen("super.bin", "rb");
        if (fd1) {
            if (fread(&s_block, sizeof(superblock), 1, fd1) != 1) {
                // Handle read error or incomplete read
                fprintf(stderr, "Error reading superblock.\n");
            }
            fclose(fd1); // Ensure the file is closed
        } else {
            fprintf(stderr, "Failed to open super.bin for reading.\n");
        }
    } else {
*/
        printf("NEW SYSTEM\n");
        superblock_init();
        root_dir_init();

//    }

    return fuse_main(argc, argv, &operations, NULL);
}