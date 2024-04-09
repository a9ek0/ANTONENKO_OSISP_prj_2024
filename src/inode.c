#include "../include/inode.h"

int find_free_inode() {
    for (int i = 2; i < 100; i++) {
        if (spblock.inode_bitmap[i] == '0') {
            spblock.inode_bitmap[i] = '1';
            return i; // Найден свободный индекс, возвращаем его
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
