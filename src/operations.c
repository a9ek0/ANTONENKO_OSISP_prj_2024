#include "../include/operations.h"

int sfsmkdir(const char *path, mode_t mode) {
    printf("MKDIR\n");

    int index = find_free_inode();

    filetype *new_folder = malloc(sizeof(filetype));
    new_folder->inum = (inode *) malloc(sizeof(inode));

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    char *rindex = strrchr(pathname, '/');

    strcpy(new_folder->name, rindex + 1);
    strcpy(new_folder->path, pathname);

    *rindex = '\0';

    if (strlen(pathname) == 0)
        strcpy(pathname, "/");

    new_folder->children = NULL;
    new_folder->num_children = 0;
    new_folder->parent = filetype_from_path(pathname);
    new_folder->num_links = 2;
    new_folder->valid = 1;
    strcpy(new_folder->test, "test");

    if (new_folder->parent == NULL)
        return -ENOENT;

    add_child(new_folder->parent, new_folder);

    strcpy(new_folder->type, "directory");

    new_folder->inum->c_time = time(NULL);
    new_folder->inum->a_time = time(NULL);
    new_folder->inum->m_time = time(NULL);
    new_folder->inum->b_time = time(NULL);

    new_folder->inum->permissions = S_IFDIR | 0777;

    new_folder->inum->size = 0;
    new_folder->inum->group_id = getgid();
    new_folder->inum->user_id = getuid();

    new_folder->inum->number = index;
    new_folder->inum->blocks = 0;

    save_contents();

    return 0;
}

int sfsgetattr(const char *path, struct stat *stat) {
    char *pathname;
    pathname = (char *) malloc(strlen(path) + 2);

    strcpy(pathname, path);

    printf("GETATTR\n");

    filetype *file_node = filetype_from_path(pathname);
    if (file_node == NULL)
        return -ENOENT;

    stat->st_uid = file_node->inum->user_id;
    stat->st_gid = file_node->inum->group_id;
    stat->st_atime = file_node->inum->a_time;
    stat->st_mtime = file_node->inum->m_time;
    stat->st_ctime = file_node->inum->c_time;
    stat->st_mode = file_node->inum->permissions;
    stat->st_nlink = file_node->num_links + file_node->num_children;
    stat->st_size = file_node->inum->size;
    stat->st_blocks = file_node->inum->blocks;

    return 0;
}

int sfsreaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("READDIR\n");

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    char *pathname = malloc(strlen(path) + 2);
    strcpy(pathname, path);

    filetype *dir_node = filetype_from_path(pathname);

    if (dir_node == NULL) {
        return -ENOENT;
    } else {
        dir_node->inum->a_time = time(NULL);
        for (int i = 0; i < dir_node->num_children; i++) {
            printf(":%s:\n", dir_node->children[i]->name);
            filler(buffer, dir_node->children[i]->name, NULL, 0);
        }
    }

    return 0;
}