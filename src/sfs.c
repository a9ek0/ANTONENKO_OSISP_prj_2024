#include "../include/sfs.h"

superblock spblock;
filetype *root;
filetype file_array[31];

int save_contents() {
    // Allocate space for the queue to store filetypes
    filetype *queue = malloc(sizeof(filetype) * 60);
    int front = 0;
    int rear = 0;
    queue[0] = *root;
    int index = 0;
    tree_to_array(queue, &front, &rear, &index);

    // Open file for storing file structure
    FILE *file_structure = fopen("file_structure.bin", "wb");
    // Open file for storing superblock
    FILE *superblock_file = fopen("super.bin", "wb");

    // Write array of filetypes to file_structure
    fwrite(file_array, sizeof(filetype) * 31, 1, file_structure);
    // Write superblock structure to superblock_file
    fwrite(&spblock, sizeof(superblock), 1, superblock_file);

    fclose(file_structure);
    fclose(superblock_file);

    free(queue);
    return 0;
}

void root_dir_init() {
    spblock.inode_bitmap[1] = 1; // Mark inode 1 as used
    root = malloc(sizeof(filetype));

    // Set initial values for root directory
    strcpy(root->path, "/");
    strcpy(root->name, "/");
    strcpy(root->test, "test");
    strcpy(root->type, "directory");

    // Allocate memory for inode and set its properties
    root->inum = malloc(sizeof(inode));
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
    root->inum->number = index;
    root->inum->blocks = 0;

    // Save the contents to disk
    save_contents();
}