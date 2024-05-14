#include "../include/superblock.h"

superblock s_block;

void superblock_init() {
    // Initialize all data block bits to '0' indicating they are free
    memset(s_block.data_bitmap, '0', sizeof(s_block.data_bitmap));
    // Initialize all inode bits to '0' indicating they are free
    memset(s_block.inode_bitmap, '0', sizeof(s_block.inode_bitmap));
}

int find_free_db() {
    for (int i = 1; i < 100; i++) {
        if (s_block.data_bitmap[i] == '0') {
            s_block.data_bitmap[i] = '1';
            return i; // Free data block found, return its index
        }
    }
    return -1; // No free data block found
}