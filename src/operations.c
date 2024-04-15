#include "../include/operations.h"

int sfs_mkdir(const char *path, mode_t mode) {
    printf("Creating directory: %s\n", path);

    // Find a free inode
    int index = find_free_inode();
    if (index == -1) {
        return -ENOSPC; // No space left on device
    }

    // Allocate memory for the new folder
    filetype *new_folder = malloc(sizeof(filetype));
    new_folder->inum = malloc(sizeof(inode));

    // Copy the path and extract the folder name
    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);
    char *folder_name = strrchr(pathname, '/');
    strcpy(new_folder->name, folder_name + 1);
    *folder_name = '\0';

    // Set the folder path
    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }
    strcpy(new_folder->path, pathname);

    // Get the parent folder
    new_folder->parent = filetype_from_path(pathname);
    if (new_folder->parent == NULL) {
        return -ENOENT; // No such file or directory
    }

    // Add the new folder as a child of the parent folder
    add_child(new_folder->parent, new_folder);

    // Set the folder properties
    new_folder->num_links = 2;
    new_folder->valid = 1;
    strcpy(new_folder->type, "directory");

    // Set the inode properties
    inode *new_inode = new_folder->inum;
    new_inode->c_time = time(NULL);
    new_inode->a_time = time(NULL);
    new_inode->m_time = time(NULL);
    new_inode->b_time = time(NULL);
    new_inode->permissions = S_IFDIR | 0777;
    new_inode->size = 0;
    new_inode->group_id = getgid();
    new_inode->user_id = getuid();
    new_inode->number = index;
    new_inode->blocks = 0;

    // Save the changes
    save_contents();

    return 0;
}

int sfs_getattr(const char *path, struct stat *stat) {
    printf("Getting attributes for: %s\n", path);

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    filetype *file_node = filetype_from_path(pathname);
    if (file_node == NULL) {
        return -ENOENT; // No such file or directory
    }

    inode *file_inode = file_node->inum;
    stat->st_uid = file_inode->user_id;
    stat->st_gid = file_inode->group_id;
    stat->st_atime = file_inode->a_time;
    stat->st_mtime = file_inode->m_time;
    stat->st_ctime = file_inode->c_time;
    stat->st_mode = file_inode->permissions;
    stat->st_nlink = file_node->num_links + file_node->num_children;
    stat->st_size = file_inode->size;
    stat->st_blocks = file_inode->blocks;

    return 0;
}

int sfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("Reading directory: %s\n", path);

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    filetype *dir_node = filetype_from_path(pathname);

    if (dir_node == NULL) {
        return -ENOENT; // No such file or directory
    }

    dir_node->inum->a_time = time(NULL);

    for (int i = 0; i < dir_node->num_children; i++) {
        printf(":%s:\n", dir_node->children[i]->name);
        filler(buffer, dir_node->children[i]->name, NULL, 0);
    }

    return 0;
}

int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("Creating file: %s\n", path);

    int index = find_free_inode();
    if (index == -1) {
        return -ENOSPC; // No space left on device
    }

    filetype *new_file = malloc(sizeof(filetype));

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    char *folder_name = strrchr(pathname, '/');
    strcpy(new_file->name, folder_name + 1);
    *folder_name = '\0';

    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    strcpy(new_file->path, pathname);

    new_file->num_children = 0;
    new_file->parent = filetype_from_path(pathname);
    if (new_file->parent == NULL) {
        return -ENOENT; // No such file or directory
    }

    add_child(new_file->parent, new_file);

    strcpy(new_file->type, "file");

    inode *new_inode = malloc(sizeof(inode));
    new_inode->number = index;
    new_inode->blocks = 0;
    new_inode->size = 0;
    new_inode->permissions = mode;
    new_inode->user_id = getuid();
    new_inode->group_id = getgid();
    new_inode->a_time = time(NULL);
    new_inode->m_time = time(NULL);
    new_inode->c_time = time(NULL);
    new_inode->b_time = time(NULL);

    for (int i = 0; i < 16; i++) {
        new_inode->datablocks[i] = find_free_db();
    }

    new_file->inum = new_inode;

    save_contents();

    return 0;
}

int sfs_rmdir(const char *path) {
    printf("Removing directory: %s\n", path);

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    char *folder_name = strrchr(pathname, '/');
    char *folder_delete = malloc(strlen(folder_name + 1) + 2);
    strcpy(folder_delete, folder_name + 1);
    *folder_name = '\0';

    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    filetype *parent = filetype_from_path(pathname);
    if (parent == NULL) {
        return -ENOENT; // No such file or directory
    }

    int index = 0;
    while (index < parent->num_children) {
        if (strcmp(parent->children[index]->name, folder_delete) == 0) {
            break;
        }
        index++;
    }
    if (index == parent->num_children) {
        return -ENOENT; // No such file or directory
    }
    if (parent->children[index]->num_children != 0) {
        return -ENOTEMPTY; // Directory not empty
    }
    for (int i = index + 1; i < parent->num_children; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    parent->num_children -= 1;

    save_contents();

    return 0;
}

int sfs_rm(const char *path) {
    printf("Removing file: %s\n", path);

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    char *folder_name = strrchr(pathname, '/');
    char *folder_delete = malloc(strlen(folder_name + 1) + 2);
    strcpy(folder_delete, folder_name + 1);
    *folder_name = '\0';

    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    filetype *parent = filetype_from_path(pathname);
    if (parent == NULL) {
        return -ENOENT; // No such file or directory
    }

    int index = 0;
    while (index < parent->num_children) {
        if (strcmp(parent->children[index]->name, folder_delete) == 0) {
            break;
        }
        index++;
    }
    if (index == parent->num_children) {
        return -ENOENT; // No such file or directory
    }
    if (parent->children[index]->num_children != 0) {
        return -ENOTEMPTY; // Directory not empty
    }
    for (int i = index + 1; i < parent->num_children; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    parent->num_children -= 1;

    save_contents();

    return 0;
}

int sfs_open(const char *path, struct fuse_file_info *fi) {
    printf("Opening file: %s\n", path);

    char *pathname = malloc(strlen(path) + 1);
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    return 0;
}

int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("Reading file: %s\n", path);

    char *pathname = malloc(strlen(path) + 1);
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    } else {
        char *str = malloc((file->inum->size) + 1);
        memset(str, 0, (file->inum->size) + 1);

        printf(":%d:\n", file->inum->size);

        int i;
        for (i = 0; i < (file->inum->blocks) - 1; i++) {
            strncat(str, &s_block.datablocks[block_size * (file->inum->datablocks[i])], 1024);
            printf("--> %s", str);
        }
        strncat(str, &s_block.datablocks[block_size * (file->inum->datablocks[i])], (file->inum->size) % 1024);
        printf("--> %s", str);

        strncpy(buf, str + offset, size);
        free(str);
    }

    return file->inum->size;
}

int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("Writing to file: %s\n", path);

    char *pathname = malloc(strlen(path) + 1);
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    int indexno = file->inum->blocks - 1;

    if (file->inum->size == 0) {
        strcpy(&s_block.datablocks[block_size * (file->inum->datablocks[0])], buf);
        file->inum->size = strlen(buf);
        file->inum->blocks++;
    } else {
        int currblk = file->inum->blocks - 1;
        int len1 = 1024 - (file->inum->size % 1024);

        if (len1 >= strlen(buf)) {
            strcat(&s_block.datablocks[block_size * (file->inum->datablocks[currblk])], buf);
            file->inum->size += strlen(buf);
            printf("---> %s\n", &s_block.datablocks[block_size * (file->inum->datablocks[currblk])]);
        } else {
            char *cpystr = malloc(1024 * sizeof(char));
            strncpy(cpystr, buf, len1 - 1);
            strcat(&s_block.datablocks[block_size * (file->inum->datablocks[currblk])], cpystr);
            strcpy(cpystr, buf);
            strcpy(&s_block.datablocks[block_size * (file->inum->datablocks[currblk + 1])], (cpystr + len1 - 1));
            file->inum->size += strlen(buf);
            printf("---> %s\n", &s_block.datablocks[block_size * (file->inum->datablocks[currblk])]);
            file->inum->blocks++;
        }
    }

    save_contents();

    return strlen(buf);
}

int sfs_rename(const char* from, const char* to) {
    printf("Renaming file/directory: %s to %s\n", from, to);

    char *pathname = malloc(strlen(from) + 2);
    strcpy(pathname, from);

    char *rindex1 = strrchr(pathname, '/');

    filetype *file = filetype_from_path(pathname);

    *rindex1 = '\0';

    char *pathname2 = malloc(strlen(to) + 2);
    strcpy(pathname2, to);

    char *rindex2 = strrchr(pathname2, '/');

    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    strcpy(file->name, rindex2 + 1);
    strcpy(file->path, to);

    printf(":%s:\n", file->name);
    printf(":%s:\n", file->path);

    save_contents();

    return 0;
}

int sfs_truncate(const char *path, off_t size) {
    // This function is currently a placeholder and does not perform any operations.
    return 0;
}

int sfs_access(const char *path, int mask) {
    // This function is currently a placeholder and does not perform any operations.
    return 0;
}
