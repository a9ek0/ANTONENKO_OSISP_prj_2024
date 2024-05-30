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

    new_folder->children = NULL;

    // Add the new folder as a child of the parent folder
    add_child(new_folder->parent, new_folder);

    // Set the folder properties
    new_folder->num_children = 0;
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

    save_system_state();

    free(pathname);

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
    new_file->valid = 1;

    save_system_state();

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

    save_system_state();

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

    save_system_state();

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
    if (offset + size > (size_t)file->inum->size) {
        read_size = file->inum->size - offset;
        if ((ssize_t)read_size < 0) {
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
        strncat(str, &s_block.data_blocks[block_offset], copy_size);
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
    if (buf_len > (size_t)INT_MAX - file->inum->size) {
        return -EFBIG; // File too large
    }

    if (file->inum->size == 0) {
        strncpy(&s_block.data_blocks[block_size * (file->inum->datablocks[0])], buf, buf_len);
        file->inum->size = (int) buf_len; // Safe cast after size check
        file->inum->blocks++;
    } else {
        int currblk = file->inum->blocks - 1;
        size_t len1 = 1024 - (file->inum->size % 1024);

        if (len1 >= buf_len) {
            strncat(&s_block.data_blocks[block_size * (file->inum->datablocks[currblk])], buf, buf_len);
            file->inum->size += (int) buf_len; // Safe cast after size check
        } else {
            char *cpystr = malloc(1024 * sizeof(char));
            if (!cpystr) {
                return -ENOMEM; // Not enough memory
            }
            strncpy(cpystr, buf, len1);
            strncat(&s_block.data_blocks[block_size * (file->inum->datablocks[currblk])], cpystr, len1);
            strncpy(&s_block.data_blocks[block_size * (file->inum->datablocks[currblk + 1])], buf + len1,
                    buf_len - len1);
            file->inum->size += (int) buf_len;
            file->inum->blocks++;
            free(cpystr);
        }
    }

    save_system_state();

    return (int) buf_len;
}

int sfs_rename(const char *from, const char *to) {
    printf("Renaming file/directory from %s to %s\n", from, to);

    filetype *from_node = filetype_from_path(from);
    if (from_node == NULL) {
        return -ENOENT;
    }

    char *dest_name = get_file_name(to);

    char *path_to = get_file_path(to);

    char *path_from = get_file_path(from);

    filetype *to_dir = filetype_from_path(to);
    if (to_dir == NULL) {
        filetype *parent_dir_from = filetype_from_path(path_from);
        filetype *parent_dir_to = filetype_from_path(path_to);
        if (parent_dir_from == NULL || parent_dir_to == NULL) {
            return -ENOENT;
        }

        for (int i = 0; i < parent_dir_from->num_children; i++) {
            if (strcmp(parent_dir_from->children[i]->name, get_file_name(from)) == 0) {
                if (strcmp(get_file_name(to), get_file_name(from)) != 0)
                    strcpy(parent_dir_from->children[i]->name, get_file_name(to));
                strcpy(parent_dir_from->children[i]->path, get_file_path(to));
                add_child(parent_dir_to, parent_dir_from->children[i]);

                for (int j = i + 1; j < parent_dir_from->num_children; j++) {
                    parent_dir_from->children[j - 1] = parent_dir_from->children[j];
                }

                parent_dir_from->num_children--;

                break;
            }
        }


        printf("To - %s %s : From - %s %s\n", path_to, get_file_name(to), path_from,
               get_file_name(from));

        save_system_state();

        return 0;
    }


    filetype *parent_dir_from = filetype_from_path(get_file_path(from));
    filetype *existing_node = NULL;
    for (int i = 0; i < parent_dir_from->num_children; i++) {
        if (strcmp(parent_dir_from->children[i]->name, dest_name) == 0) {
            existing_node = parent_dir_from->children[i];
            break;
        }
    }


    if (existing_node != NULL) {
        if (strcmp(existing_node->type, "directory") != 0) {
            sfs_rm(to);
        }
    }


    strcpy(from_node->name, dest_name);
    strcpy(from_node->path, to);

    save_system_state();

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

    if (tv[1].tv_nsec != UTIME_OMIT) {
        file->inum->m_time = (tv[1].tv_nsec == UTIME_NOW) ? currentTime : tv[1].tv_sec;
    }

    save_system_state();

    return 0;
}

