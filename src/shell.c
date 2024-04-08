#include "sfs.h"

// gcc shell.c -o FS `pkg-config fuse --cflags --libs`
// ./shell -f /home/alexander/mnt
// fusermount -u /home/alexander/mnt

int main(int argc, char *argv[]) {
    FILE *fd = fopen("file_structure.bin", "rb");
    if (fd) {
        printf("SYSTEM RESTORED\n");
        fread(&file_array, sizeof(filetype) * 31, 1, fd);

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
        fread(&spblock, sizeof(superblock), 1, fd1);
    } else {
        printf("NEW SYSTEM\n");
        superblock_init();
        root_dir_init();
    }

    return fuse_main(argc, argv, &operations, NULL);
}