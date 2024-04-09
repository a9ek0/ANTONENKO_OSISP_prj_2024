#ifndef ANTONENKO_OSISP_PRJ_2024_SUPERBLOCK_H
#define ANTONENKO_OSISP_PRJ_2024_SUPERBLOCK_H

#include <string.h>

#define block_size 1024

typedef struct superblock {
    char datablocks[block_size * 100];  // общее количество блоков данных
    char data_bitmap[105];              // массив номеров блоков данных, которые доступны
    char inode_bitmap[105];             // массив номеров индексных узлов, которые доступны
} superblock;

extern superblock spblock;

void superblock_init();
int find_free_db();

#endif //ANTONENKO_OSISP_PRJ_2024_SUPERBLOCK_H
