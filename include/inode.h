#ifndef ANTONENKO_OSISP_PRJ_2024_INODE_H
#define ANTONENKO_OSISP_PRJ_2024_INODE_H

#include "superblock.h"
#include "filetype.h"

typedef struct filetype filetype;
typedef struct inode {
    int datablocks[16];        // номера блоков данных
    int number;                // номер inode
    int blocks;                // количество блоков данных
    int size;                  // размер файла/каталога
    mode_t permissions;        // права доступа
    uid_t user_id;             // идентификатор пользователя
    gid_t group_id;            // идентификатор группы
    time_t a_time;             // время доступа
    time_t m_time;             // время изменения
    time_t c_time;             // время изменения состояния
    time_t b_time;             // время создания
} inode;

int find_free_inode();
void add_child(filetype *parent, filetype *child);

#endif //ANTONENKO_OSISP_PRJ_2024_INODE_H
