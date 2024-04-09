#include "../include/superblock.h"

void superblock_init() {
    memset(spblock.data_bitmap, '0', 100 * sizeof(char));
    memset(spblock.inode_bitmap, '0', 100 * sizeof(char));
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
