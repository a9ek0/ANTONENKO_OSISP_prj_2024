#include "../include/sfs.h"

superblock spblock;
filetype *root;
filetype file_array[MAX_FILES];
filesystem_t fs;


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
    save_contents();
}

int save_contents() {
    printf("SAVING\n");
    filetype *queue = malloc(sizeof(filetype) * 60);
    int front = 0;
    int rear = 0;
    queue[0] = *root;
    int index = 0;
    tree_to_array(queue, &front, &rear, &index);

    FILE *fd = fopen("file_structure.bin", "wb");

    FILE *fd1 = fopen("super.bin", "wb");

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

    fwrite(spblock.datablocks, sizeof(spblock.datablocks[0]),
           sizeof(spblock.datablocks) / sizeof(spblock.datablocks[0]), fd1);

    fwrite(spblock.data_bitmap, sizeof(spblock.data_bitmap[0]),
           sizeof(spblock.data_bitmap) / sizeof(spblock.data_bitmap[0]), fd1);

    fwrite(spblock.inode_bitmap, sizeof(spblock.inode_bitmap[0]),
           sizeof(spblock.inode_bitmap) / sizeof(spblock.inode_bitmap[0]), fd1);
//    fwrite(&spblock, sizeof(superblock), 1, fd1);

    fclose(fd);
    fclose(fd1);

    return 0;
}


// Recursive helper function to save filetypes (internal use only)
int save_filetype(FILE *fp, const filetype *ft) {
    if (ft == NULL) {
        return 0; // Base case: null pointer
    }

    // Write filetype itself
    size_t written = fwrite(ft, sizeof(*ft), 1, fp);
    if (written != 1) {
        perror("fwrite (filetype)");
        return -1;
    }

    // Recursively save children
    for (int i = 0; i < ft->num_children; i++) {
        int save_result = save_filetype(fp, ft->children[i]);
        if (save_result != 0) {
            return save_result; // Propagate error
        }
    }

    return 0;
}

int restore_contents(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    // Read superblock
    size_t read = fread(&spblock, sizeof(spblock), 1, fp);
    if (read != 1) {
        perror("fread (superblock)");
        fclose(fp);
        return -1;
    }

    // Allocate memory for root (assuming at least one filetype)
    root = malloc(sizeof(filetype));
    if (root == NULL) {
        perror("malloc (root)");
        fclose(fp);
        return -1;
    }

    // Load root filetype
    read = fread(root, sizeof(filetype), 1, fp);
    if (read != 1) {
        perror("fread (root)");
        free(root);
        fclose(fp);
        return -1;
    }

    // Recursive function to load filetypes
    int load_result = load_filetypes(fp, root);
    if (load_result != 0) {
        fclose(fp);
        return load_result; // Propagate error
    }

    // Read file array
    read = fread(file_array, sizeof(filetype), MAX_FILES, fp);
    if (read != MAX_FILES) {
        perror("fread (file_array)");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int load_filetypes(FILE *fp, filetype *ft) {
    if (ft == NULL) {
        return 0; // Base case: null pointer, considered successful
    }

    // Allocate memory for child pointers
    ft->children = malloc(ft->num_children * sizeof(filetype *));
    if (ft->children == NULL) {
        perror("malloc (children)");
        return -1; // Return error code for allocation failure
    }

    for (int i = 0; i < ft->num_children; i++) {
        ft->children[i] = malloc(sizeof(filetype));
        if (ft->children[i] == NULL) {
            perror("malloc (child)");
            // Free previously allocated memory to avoid leaks
            for (int j = 0; j < i; j++) {
                free(ft->children[j]);
            }
            free(ft->children);
            return -1; // Return error code for allocation failure
        }

        // Read the child filetype from the file
        size_t read = fread(ft->children[i], sizeof(filetype), 1, fp);
        if (read != 1) {
            perror("fread (child)");
            // Free previously allocated memory to avoid leaks
            for (int j = 0; j <= i; j++) {
                free(ft->children[j]);
            }
            free(ft->children);
            return -1; // Return error code for read failure
        }

        // Recursively load child filetypes and check for errors
        int load_result = load_filetypes(fp, ft->children[i]);
        if (load_result != 0) {
            // Propagate the error up if a child fails to load
            return load_result;
        }
    }

    return 0; // Success
}

void apply_fs_structure(filesystem_t *fs) {
    // Обновление superblock
    spblock = fs->s_block;

    // Проверка и обновление root
    if (root != NULL) {
        if (root->inum != NULL) {
            free(root->inum); // Освобождение памяти, если уже выделена
        }
        free(root); // Освобождение старой структуры root перед обновлением
    }
    root = malloc(sizeof(filetype));
    if (root != NULL) {
        *root = fs->root;
        // Для inum требуется глубокое копирование
        root->inum = malloc(sizeof(inode));
        if (root->inum != NULL) {
            *root->inum = *fs->root.inum;
        } else {
            perror("Failed to allocate memory for inode");
            free(root);
            root = NULL;
            return;
        }
    } else {
        perror("Failed to allocate memory for root");
        return;
    }

    // Обновление file_array
    for (int i = 0; i < MAX_FILES; i++) {
        file_array[i] = fs->file_array[i];
    }
}

