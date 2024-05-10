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

    save_system_state();
}

void print_superblock_details() {
    printf("Data blocks:\n");
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
    printf("\n");
}

int save_system_state() {
    FILE *fd = fopen("file_structure.bin", "wb");
    if (!fd) {
        perror("Failed to open file_structure.bin for writing");
        return -1;
    }

    serialize_filetype_to_file(root, fd);

    FILE *fd1 = fopen("super.bin", "wb");
    if (!fd1) {
        perror("Failed to open super.bin for writing");
        fclose(fd);
        return -1;
    }

    fwrite(&s_block, sizeof(superblock), 1, fd1);

    for (int i = 0; i < MAX_FILES; i++) {
        printf("%s : %s\n", file_array[i].path, file_array[i].name);
    }

    fclose(fd);
    fclose(fd1);

    return 0;
}

void restore_file_system() {
    FILE *fd = fopen("file_structure.bin", "rb");
    if (fd) {
        printf("File system restored!\n");

        root = malloc(sizeof(filetype));
        deserialize_filetype_from_file(root, fd);

        FILE *fd1 = fopen("super.bin", "rb");

        fread(&s_block, sizeof(superblock), 1, fd1);

        fclose(fd);
        fclose(fd1);
    } else {
        printf("New file system!\n");
        superblock_init();
        root_dir_init();
    }
}
