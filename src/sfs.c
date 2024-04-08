#include "sfs.h"

superblock spblock;
filetype *root;
filetype file_array[31];

void tree_to_array(filetype *queue, int *front, int *rear, int *index) {
    if (rear < front)
        return;
    if (*index > 30)
        return;

    filetype curr_node = queue[*front];
    *front += 1;
    file_array[*index] = curr_node;
    *index += 1;

    if (*index < 6) {
        if (curr_node.valid) {
            int i;
            for (i = 0; i < curr_node.num_children; i++) {
                if (*rear < *front)
                    *rear = *front;
                queue[*rear] = *(curr_node.children[i]);
                *rear += 1;
            }
            while (i < 5) {
                filetype waste_node;
                waste_node.valid = 0;
                queue[*rear] = waste_node;
                *rear += 1;
                i++;
            }
        } else {
            int i = 0;
            while (i < 5) {
                filetype waste_node;
                waste_node.valid = 0;
                queue[*rear] = waste_node;
                *rear += 1;
                i++;
            }
        }
    }

    tree_to_array(queue, front, rear, index);
}

int save_contents() {
    filetype *queue = malloc(sizeof(filetype) * 60);
    int front = 0;
    int rear = 0;
    queue[0] = *root;
    int index = 0;
    tree_to_array(queue, &front, &rear, &index);

    FILE *fd = fopen("file_structure.bin", "wb");
    FILE *fd1 = fopen("super.bin", "wb");

    fwrite(file_array, sizeof(filetype) * 31, 1, fd);
    fwrite(&spblock, sizeof(superblock), 1, fd1);

    fclose(fd);
    fclose(fd1);

    return 0;
}

filetype *filetype_from_path(char *path) {
    char curr_folder[100];
    char *path_name = malloc(strlen(path) + 2);

    strcpy(path_name, path);

    filetype *curr_node = root;

    fflush(stdin);

    if (strcmp(path_name, "/") == 0)
        return curr_node;
    if (path_name[0] != '/') {
        printf("INCORRECT PATH\n");
        exit(1);
    } else {
        path_name++;
    }

    if (path_name[strlen(path_name) - 1] == '/') {
        path_name[strlen(path_name) - 1] = '\0';
    }

    char *index;
    int flag;

    while (strlen(path_name) != 0) {
        index = strchr(path_name, '/');

        if (index != NULL) {
            strncpy(curr_folder, path_name, index - path_name);
            curr_folder[index - path_name] = '\0';

            flag = 0;
            for (int i = 0; i < curr_node->num_children; i++) {
                if (strcmp((curr_node->children)[i]->name, curr_folder) == 0) {
                    curr_node = (curr_node->children)[i];
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                return NULL;
        } else {
            strcpy(curr_folder, path_name);
            for (int i = 0; i < curr_node->num_children; i++) {
                if (strcmp((curr_node->children)[i]->name, curr_folder) == 0) {
                    curr_node = (curr_node->children)[i];
                    return curr_node;
                }
            }
            return NULL;
        }
        path_name = index + 1;
    }

    return curr_node;
}

int find_free_inode() {
    for (int i = 2; i < 100; i++) {
        if (spblock.inode_bitmap[i] == '0') {
            spblock.inode_bitmap[i] = '1';
            return i; // Найден свободный индекс, возвращаем его
        }
    }
    return -1;
}

int find_free_db() {
    for (int i = 1; i < 100; i++) {
        if (spblock.inode_bitmap[i] == '0') {
            spblock.inode_bitmap[i] = '1';
            return i; // Найден свободный блок данных, возвращаем его
        }
    }
    return -1;
}


void add_child(filetype *parent, filetype *child) {
    (parent->num_children)++;

    filetype **temp_children = parent->children;

    parent->children = realloc(parent->children, (parent->num_children) * sizeof(filetype *));

    if (parent->children == NULL) {
        parent->children = temp_children;
        return;
    }

    (parent->children)[parent->num_children - 1] = child;
}

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

void superblock_init() {
    memset(spblock.data_bitmap, '0', 100 * sizeof(char));
    memset(spblock.inode_bitmap, '0', 100 * sizeof(char));
}

void root_dir_init() {
    spblock.inode_bitmap[1] = 1;        // marking it with 0
    root = (filetype *) malloc(sizeof(filetype));

    strcpy(root->path, "/");
    strcpy(root->name, "/");
    strcpy(root->test, "test");
    strcpy(root->type, "directory");

    root->inum = (inode *) malloc(sizeof(inode));
    root->inum->permissions = S_IFDIR | 0777;
    root->inum->c_time = time(NULL);
    root->inum->a_time = time(NULL);
    root->inum->m_time = time(NULL);
    root->inum->b_time = time(NULL);
    root->inum->group_id = getgid();
    root->inum->user_id = getuid();
    root->children = NULL;
    root->parent = NULL;
    root->num_children = 0;
    root->num_links = 2;
    root->valid = 1;
    root->inum->size = 0;

    int index = find_free_inode();
    root->inum->number = index;
    root->inum->blocks = 0;

    save_contents();
}
