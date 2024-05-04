#include <limits.h>
#include "../include/operations.h"

int sfs_mkdir(const char *path, mode_t mode) {
    (void) mode; // Explicitly cast unused parameter to void to avoid warning

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
    if (folder_name != NULL) { // Check if strrchr found '/'
        strcpy(new_folder->name, folder_name + 1);
        *folder_name = '\0'; // Terminate the pathname string to remove the folder name
    }

    // Set the folder path
    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }
    strcpy(new_folder->path, pathname);

    // Get the parent folder
    new_folder->parent = filetype_from_path(pathname);
    if (new_folder->parent == NULL) {
        free(new_folder->inum);
        free(new_folder);
        free(pathname);
        return -ENOENT;
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

    save_contents();

    free(pathname); // Free allocated pathname after use

    return 0;
}

int sfs_getattr(const char *path, struct stat *stat_buf) {
    printf("Getting attributes for: %s\n", path);

    // Validate input parameters
    if (path == NULL || stat_buf == NULL) {
        return -EINVAL; // Invalid argument
    }

    char *pathname = malloc(strlen(path) + 1);
    if (pathname == NULL) {
        return -ENOMEM; // Not enough space
    }
    strcpy(pathname, path);

    // Find the file node from the path
    filetype *file_node = filetype_from_path(pathname);
    free(pathname); // Free pathname after use

    if (file_node == NULL) {
        return -ENOENT; // No such file or directory
    }

    inode *file_inode = file_node->inum;
    if (file_inode == NULL) {
        return -ENOENT; // No such file or directory
    }

    stat_buf->st_uid = file_inode->user_id;
    stat_buf->st_gid = file_inode->group_id;
    stat_buf->st_atime = file_inode->a_time;
    stat_buf->st_mtime = file_inode->m_time;
    stat_buf->st_ctime = file_inode->c_time;
    stat_buf->st_mode = file_inode->permissions;
    stat_buf->st_nlink = file_node->num_links + file_node->num_children;
    stat_buf->st_size = file_inode->size;
    stat_buf->st_blocks = file_inode->blocks;

    return 0;
}

int sfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("Reading directory: %s\n", path);

    // Explicitly cast unused parameters to void to avoid warnings
    (void) offset;
    (void) fi;

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    // Allocate memory for the pathname with correct size
    char *pathname = malloc(strlen(path) + 1);
    if (pathname == NULL) {
        return -ENOMEM; // Not enough space
    }
    strcpy(pathname, path);

    filetype *dir_node = filetype_from_path(pathname);
    free(pathname); // Free pathname after use

    if (dir_node == NULL) {
        return -ENOENT; // No such file or directory
    }

    dir_node->inum->a_time = time(NULL); // Update access time

    for (int i = 0; i < dir_node->num_children; i++) {
        printf(":%s:\n", dir_node->children[i]->name);
        filler(buffer, dir_node->children[i]->name, NULL, 0);
    }

    return 0;
}

int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // Suppress unused parameter warning by explicitly casting to void
    (void) fi;

    printf("Creating file: %s\n", path);

    int index = find_free_inode();
    if (index == -1) {
        return -ENOSPC; // No space left on device
    }

    filetype *new_file = malloc(sizeof(filetype));
    if (new_file == NULL) {
        return -ENOMEM; // Not enough memory
    }

    char *pathname = malloc(strlen(path) + 1); // +1 for null terminator
    if (pathname == NULL) {
        free(new_file); // Clean up previously allocated memory
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, path);

    char *folder_name = strrchr(pathname, '/');
    if (folder_name != NULL) {
        strcpy(new_file->name, folder_name + 1);
        *folder_name = '\0'; // Split the string at the last '/'
    } else {
        // Handle the case where strrchr might return NULL
        free(new_file);
        free(pathname);
        return -EINVAL; // Invalid argument
    }

    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    strcpy(new_file->path, pathname);

    new_file->num_children = 0;
    new_file->parent = filetype_from_path(pathname);
    if (new_file->parent == NULL) {
        free(new_file);
        free(pathname);
        return -ENOENT; // No such file or directory
    }

    add_child(new_file->parent, new_file);

    strcpy(new_file->type, "file");

    inode *new_inode = malloc(sizeof(inode));
    if (new_inode == NULL) {
        free(new_file);
        free(pathname);
        return -ENOMEM; // Not enough memory
    }
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

    free(pathname); // Free pathname after use

    return 0;
}

int sfs_rmdir(const char *path) {
    printf("Removing directory: %s\n", path);

    char *pathname = malloc(strlen(path) + 1); // +1 for null terminator
    if (!pathname) {
        return -ENOMEM;
    }
    strcpy(pathname, path);

    char *folder_name = strrchr(pathname, '/');
    if (!folder_name) {
        free(pathname);
        return -EINVAL; // Invalid path
    }

    // Correct allocation for folder_delete
    char *folder_delete = malloc(
            strlen(folder_name)); // folder_name + 1 points to the next character, so strlen already accounts for it
    if (!folder_delete) {
        free(pathname);
        return -ENOMEM;
    }
    strcpy(folder_delete, folder_name + 1);
    *folder_name = '\0'; // Terminate pathname to isolate the parent directory path

    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    filetype *parent = filetype_from_path(pathname);
    free(pathname); // Free pathname as soon as it's no longer needed

    if (parent == NULL) {
        free(folder_delete);
        return -ENOENT; // No such file or directory
    }

    int index = 0;
    while (index < parent->num_children) {
        if (strcmp(parent->children[index]->name, folder_delete) == 0) {
            break;
        }
        index++;
    }
    free(folder_delete); // Free folder_delete as soon as it's no longer needed

    if (index == parent->num_children) {
        return -ENOENT; // No such file or directory
    }
    if (parent->children[index]->num_children != 0) {
        return -ENOTEMPTY; // Directory not empty
    }
    // Shift children to remove the deleted directory
    for (int i = index + 1; i < parent->num_children; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    parent->num_children -= 1;

    save_contents();

    return 0;
}

int sfs_rm(const char *path) {
    printf("Removing file: %s\n", path);

    // Allocate memory for pathname with correct size
    char *pathname = malloc(strlen(path) + 1); // +1 for null terminator
    if (!pathname) {
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, path);

    // Find the last occurrence of '/' to isolate folder name
    char *folder_name = strrchr(pathname, '/');
    if (!folder_name) {
        free(pathname);
        return -EINVAL; // Invalid path
    }

    // Correctly allocate memory for folder_delete considering the null terminator
    char *folder_delete = malloc(strlen(folder_name)); // Corrected allocation
    if (!folder_delete) {
        free(pathname);
        return -ENOMEM; // Not enough memory
    }
    strcpy(folder_delete, folder_name + 1);
    *folder_name = '\0'; // Modify pathname to get the parent directory

    // Handle root directory case
    if (strlen(pathname) == 0) {
        strcpy(pathname, "/");
    }

    // Find the parent directory from the modified pathname
    filetype *parent = filetype_from_path(pathname);
    free(pathname); // Free pathname as it's no longer needed

    if (parent == NULL) {
        free(folder_delete);
        return -ENOENT; // No such file or directory
    }

    // Search for the file to delete among the parent's children
    int index = 0;
    while (index < parent->num_children) {
        if (strcmp(parent->children[index]->name, folder_delete) == 0) {
            break;
        }
        index++;
    }
    free(folder_delete); // Free folder_delete as it's no longer needed

    // If file not found among children
    if (index == parent->num_children) {
        return -ENOENT; // No such file or directory
    }

    // Check if the target is not a directory or if it's empty
    if (parent->children[index]->num_children != 0) {
        return -ENOTEMPTY; // Directory not empty
    }

    // Remove the file from the parent's children array
    for (int i = index + 1; i < parent->num_children; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    parent->num_children -= 1;

    save_contents();

    return 0;
}

int sfs_open(const char *path, struct fuse_file_info *fi) {
    printf("Opening file: %s\n", path);

    // Explicitly cast unused parameter to void
    (void) fi;

    char *pathname = malloc(strlen(path) + 1);
    if (!pathname) {
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    free(pathname); // Free pathname as soon as it's no longer needed
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    return 0;
}

int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("Reading file: %s\n", path);

    // Explicitly cast unused parameter to void
    (void) fi;

    char *pathname = malloc(strlen(path) + 1);
    if (!pathname) {
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    free(pathname); // Free pathname as soon as it's no longer needed
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    // Ensure that the requested read does not exceed the file size
    size_t read_size = size;
    if (offset + size > file->inum->size) {
        read_size = file->inum->size - offset;
        if (read_size < 0) {
            return 0; // Attempt to read beyond the end of the file
        }
    }

    char *str = malloc(file->inum->size + 1);
    if (!str) {
        return -ENOMEM; // Not enough memory
    }
    memset(str, 0, file->inum->size + 1);

    // Simulate reading from the file
    for (int i = 0; i < file->inum->blocks; i++) {
        size_t block_offset = block_size * file->inum->datablocks[i];
        size_t copy_size = (i < file->inum->blocks - 1) ? block_size : file->inum->size % block_size;
        strncat(str, &s_block.datablocks[block_offset], copy_size);
    }

    strncpy(buf, str + offset, read_size);
    free(str);

    return read_size;
}

int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("Writing to file: %s\n", path);

    // Explicitly cast unused parameters to void to avoid warnings
    (void) size;
    (void) offset;
    (void) fi;

    char *pathname = malloc(strlen(path) + 1);
    if (!pathname) {
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, path);

    filetype *file = filetype_from_path(pathname);
    free(pathname); // Free pathname as soon as it's no longer needed
    if (file == NULL) {
        return -ENOENT; // No such file or directory
    }

    size_t buf_len = strlen(buf);
    // Ensure that the file size does not exceed the maximum value for its type
    if (buf_len > INT_MAX - file->inum->size) {
        return -EFBIG; // File too large
    }

    if (file->inum->size == 0) {
        strncpy(&s_block.datablocks[block_size * (file->inum->datablocks[0])], buf, buf_len);
        file->inum->size = (int) buf_len; // Safe cast after size check
        file->inum->blocks++;
    } else {
        int currblk = file->inum->blocks - 1;
        size_t len1 = 1024 - (file->inum->size % 1024);

        if (len1 >= buf_len) {
            strncat(&s_block.datablocks[block_size * (file->inum->datablocks[currblk])], buf, buf_len);
            file->inum->size += (int) buf_len; // Safe cast after size check
        } else {
            char *cpystr = malloc(1024 * sizeof(char));
            if (!cpystr) {
                return -ENOMEM; // Not enough memory
            }
            strncpy(cpystr, buf, len1);
            strncat(&s_block.datablocks[block_size * (file->inum->datablocks[currblk])], cpystr, len1);
            strncpy(&s_block.datablocks[block_size * (file->inum->datablocks[currblk + 1])], buf + len1,
                    buf_len - len1);
            file->inum->size += (int) buf_len; // Safe cast after size check
            file->inum->blocks++;
            free(cpystr);
        }
    }

    save_contents();

    return (int) buf_len; // Safe cast after size check
}

int sfs_rename(const char *from, const char *to) {
    printf("Renaming file/directory: %s to %s\n", from, to);

    // Allocate memory and check for success
    char *pathname = malloc(strlen(from) + 1);
    if (!pathname) {
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname, from);

    // Find the last occurrence of '/'
    char *rindex1 = strrchr(pathname, '/');
    if (!rindex1) {
        free(pathname); // Free allocated memory before returning
        return -EINVAL; // Invalid argument
    }

    // Locate the file to rename
    filetype *file = filetype_from_path(pathname);
    if (file == NULL) {
        free(pathname); // Ensure pathname is freed in case of error
        return -ENOENT; // No such file or directory
    }

    // Allocate memory for the new path and check for success
    char *pathname2 = malloc(strlen(to) + 1); // Corrected allocation size
    if (!pathname2) {
        free(pathname); // Ensure pathname is freed in case of error
        return -ENOMEM; // Not enough memory
    }
    strcpy(pathname2, to);

    // Find the last occurrence of '/' in the new path
    char *rindex2 = strrchr(pathname2, '/');
    if (!rindex2) {
        free(pathname);
        free(pathname2); // Free allocated memory before returning
        return -EINVAL; // Invalid argument
    }

    // Perform the renaming operation
    strcpy(file->name, rindex2 + 1);
    strcpy(file->path, to);

    printf(":%s:\n", file->name);
    printf(":%s:\n", file->path);

    // Free allocated memory
    free(pathname);
    free(pathname2);

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

int sfs_utimens(const char *path, const struct timespec tv[2]) {
    filetype *file = filetype_from_path(path);
    if (file == NULL) {
        return -ENOENT; // File not found
    }

    time_t currentTime = time(NULL); // Fetch current time once, if needed

    // Update access time
    if (tv[0].tv_nsec != UTIME_OMIT) {
        file->inum->a_time = (tv[0].tv_nsec == UTIME_NOW) ? currentTime : tv[0].tv_sec;
    }

    // Update modification time
    if (tv[1].tv_nsec != UTIME_OMIT) {
        file->inum->m_time = (tv[1].tv_nsec == UTIME_NOW) ? currentTime : tv[1].tv_sec;
    }

    save_contents();

    return 0;
}