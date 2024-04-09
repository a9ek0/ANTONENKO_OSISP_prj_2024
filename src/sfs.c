#include "../include/sfs.h"

superblock spblock;
filetype *root;
filetype file_array[31];

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
