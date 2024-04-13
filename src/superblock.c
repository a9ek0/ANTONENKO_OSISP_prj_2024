#include "../include/superblock.h"

void superblock_init() {
    memset(s_block.data_bitmap, '0', sizeof(spblock.data_bitmap));
    memset(s_block.inode_bitmap, '0', sizeof(spblock.inode_bitmap));
}

int find_free_db() {
    for (int i = 1; i < 100; i++) {
        if (s_block.inode_bitmap[i] == '0') {
            s_block.inode_bitmap[i] = '1';
            return i; // Free data block found, return its index
        }
    }
    return -1; // No free data block found
}