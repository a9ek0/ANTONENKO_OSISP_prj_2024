#ifndef SFS_H
#define SFS_H

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define block_size 1024

typedef struct superblock {
    char datablocks[block_size * 100];  // общее количество блоков данных
    char data_bitmap[105];              // массив номеров блоков данных, которые доступны
    char inode_bitmap[105];             // массив номеров индексных узлов, которые доступны
} superblock;

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

typedef struct filetype {
    int valid;
    char test[10];
    char path[100];
    char name[100];            // имя
    inode *inum;               // указатель на inode
    struct filetype **children;
    int num_children;
    int num_links;
    struct filetype *parent;
    char type[20];             // расширение файла
} filetype;

extern superblock spblock;
extern filetype *root;
extern filetype file_array[31];

void superblock_init();
void root_dir_init();
void tree_to_array(filetype *queue, int *front, int *rear, int *index);
int save_contents();
filetype *filetype_from_path(char *path);
int find_free_inode();
int find_free_db();
void add_child(filetype *parent, filetype *child);
int sfsmkdir(const char *path, mode_t mode);
int sfsgetattr(const char *path, struct stat *stat);
int sfsreaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi );

static struct fuse_operations operations =
        {
                .mkdir=sfsmkdir,
                .readdir=sfsreaddir,
                .getattr=sfsgetattr,
        };

#endif /* SFS_H */