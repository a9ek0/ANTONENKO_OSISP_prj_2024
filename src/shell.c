#include "../include/sfs.h"
// gcc shell.c -o FS `pkg-config fuse --cflags --libs`

//MAIN FS
// ./shell -f /home/alexander/mnt
// fusermount -u /home/alexander/mnt

//SDCARD
// sudo mount /dev/mmcblk0p1 /home/alexander/sdCard/
// sudo ./shell -f -o allow_other /home/alexander/sdCard/mnt/
// sudo fusermount -u /home/alexander/sdCard/mnt/

#include <stdio.h>
#include "../include/sfs.h"

#define MAX_FILES 31

int main(int argc, char *argv[]) {

    FILE *fd = fopen("file_structure.bin", "rb");
    if (fd) {
        //root_dir_init();
        printf("LOADING\n");
        fread(&file_array, sizeof(filetype) * 31, 1, fd);

        inode *inodes = malloc(sizeof(inode) * 31);
        for (int i = 0; i < 31; i++) {
            fread(&inodes[i], sizeof(inode), 1, fd);
            if (inodes[i].c_time != -1) {
                file_array[i].inum = &inodes[i];
                continue;
            }
            file_array[i].inum = NULL;
        }

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
        root->inum = &inodes[0];

        FILE *fd1 = fopen("super.bin", "rb");

        fread(s_block.datablocks, sizeof(s_block.datablocks[0]),
              sizeof(s_block.datablocks) / sizeof(s_block.datablocks[0]), fd1);

        fread(s_block.data_bitmap, sizeof(s_block.data_bitmap[0]),
              sizeof(s_block.data_bitmap) / sizeof(s_block.data_bitmap[0]), fd1);

        fread(s_block.inode_bitmap, sizeof(s_block.inode_bitmap[0]),
              sizeof(s_block.inode_bitmap) / sizeof(s_block.inode_bitmap[0]), fd1);
        //fread(&s_block, sizeof(superblock), 1, fd1);

    } else {
        superblock_init();
        root_dir_init();
    }


    //  printf("file_structure.bin not found. Initializing new file system.\n");
    //  superblock_init();
    //  root_dir_init();


    return fuse_main(argc, argv, &operations, NULL);
}