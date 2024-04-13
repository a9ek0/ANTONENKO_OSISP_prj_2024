#include "../include/inode.h"

int find_free_inode() {
    for (int i = 2; i < 100; i++) {
        if (spblock.inode_bitmap[i] == '0') {
            spblock.inode_bitmap[i] = '1';
            return i; // Free inode index found, return it
        }
    }
    return -1; // No free inode found
}

void add_child(filetype *parent, filetype *child) {
    parent->num_children++;

    filetype **new_children = realloc(parent->children, parent->num_children * sizeof(filetype *));
    if (new_children == NULL) {
        // Failed to allocate memory, revert back to original children array
        parent->num_children--;
        return;
    }

    parent->children = new_children;
    parent->children[parent->num_children - 1] = child;
}