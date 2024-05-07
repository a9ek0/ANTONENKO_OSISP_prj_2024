#include "../include/fs_init.h"

//superblock spblock;
filetype *root;
filetype file_array[MAX_FILES];


void root_dir_init() {
    s_block.inode_bitmap[1] = 1; // Mark inode 1 as used

    root = malloc(sizeof(filetype));
    if (!root) {
        perror("Failed to allocate memory for root");
        return; // Early return if memory allocation fails
    }

    // Set initial values for root directory
    strcpy(root->path, "/");
    strcpy(root->name, "/");
    strcpy(root->type, "directory");

    root->inum = malloc(sizeof(inode));
    if (!root->inum) {
        perror("Failed to allocate memory for inode");
        free(root);
        return; // Early return if memory allocation fails
    }

    // Set inode properties
    root->inum->permissions = S_IFDIR | 0777;
    time_t currentTime = time(NULL);
    root->inum->c_time = currentTime;
    root->inum->a_time = currentTime;
    root->inum->m_time = currentTime;
    root->inum->b_time = currentTime;
    root->inum->group_id = getgid();
    root->inum->user_id = getuid();
    root->children = NULL;
    root->parent = NULL;
    root->num_children = 0;
    root->num_links = 2;
    root->valid = 1;
    root->inum->size = 0;

    // Find a free inode index and assign it to root
    int index = find_free_inode();
    if (index == -1) {
        perror("Failed to find a free inode");
        free(root->inum);
        free(root);
        return;
    }
    root->inum->number = index;
    root->inum->blocks = 0;

    // Attempt to save the contents to disk
    save_system_state();
}

int save_system_state() {
    filetype *queue = malloc(sizeof(filetype) * 60);
    int front = 0;
    int rear = 0;
    queue[0] = *root;
    int index = 0;
    serialize_array(queue, &front, &rear, &index);

    FILE *fd = fopen("file_structure.bin", "wb");

    FILE *fd1 = fopen("super.bin", "wb");

   /* for (int i = 0; i < MAX_FILES; i++) {
        printf("%s\n", file_array[i].name);
    }*/

    fwrite(file_array, sizeof(filetype) * 31, 1, fd);

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_array[i].valid) {
            fwrite(file_array[i].inum, sizeof(inode), 1, fd);
        } else {
            inode empty_inode = {0};
            empty_inode.c_time = -1;
            fwrite(&empty_inode, sizeof(inode), 1, fd);
        }
    }


    /*printf("Data blocks:\n");
    for (int i = 0; i < sizeof(s_block.data_blocks); i++) {
        printf("%c.", s_block.data_blocks[i]);
    }
    printf("\n");

    printf("Data Bitmap:\n");
    for (int i = 0; i < sizeof(s_block.data_bitmap); i++) {
        printf("%c.", s_block.data_bitmap[i]);
    }
    printf("\n");

    printf("Inode Bitmap:\n");
    for (int i = 0; i < sizeof(s_block.inode_bitmap); i++) {
        printf("%c.", s_block.inode_bitmap[i]);
    }
    printf("\n");*/

   fwrite(&s_block, sizeof(superblock), 1, fd1);

    fclose(fd);
    fclose(fd1);

    return 0;
}
