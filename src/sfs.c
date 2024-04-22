#include "../include/sfs.h"

superblock spblock;
filetype *root;
filetype file_array[31];

int save_contents() {
    // Allocate space for the queue to store filetypes
    filetype *queue = malloc(sizeof(filetype) * 60);
    if (!queue) {
        perror("Failed to allocate memory for queue");
        return -1; // Return an error code if memory allocation fails
    }

    queue[0] = *root;
    int front = 0, rear = 0, index = 0;
    tree_to_array(queue, &front, &rear, &index); // Assuming this function populates the queue

    // Open file for storing file structure
    FILE *file_structure = fopen("file_structure.bin", "wb");
    if (!file_structure) {
        perror("Failed to open file_structure.bin for writing");
        free(queue);
        return -1;
    }

    // Open file for storing superblock
    FILE *superblock_file = fopen("super.bin", "wb");
    if (!superblock_file) {
        perror("Failed to open super.bin for writing");
        fclose(file_structure);
        free(queue);
        return -1;
    }

    // Write array of filetypes to file_structure
    if (fwrite(file_array, sizeof(filetype) * 31, 1, file_structure) != 1) {
        perror("Failed to write to file_structure.bin");
    }

    // Write superblock structure to superblock_file
    if (fwrite(&spblock, sizeof(superblock), 1, superblock_file) != 1) {
        perror("Failed to write to super.bin");
    }

    fclose(file_structure);
    fclose(superblock_file);
    free(queue);
    return 0; // Return 0 to indicate success
}

void root_dir_init() {
    spblock.inode_bitmap[1] = 1; // Mark inode 1 as used

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
    if (save_contents() != 0) {
        perror("Failed to save contents");
    }
}