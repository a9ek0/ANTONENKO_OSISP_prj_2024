#ifndef ANTONENKO_OSISP_PRJ_2024_FILETYPE_H
#define ANTONENKO_OSISP_PRJ_2024_FILETYPE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inode.h"

typedef struct inode inode;
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

extern filetype *root;
extern filetype file_array[31];

filetype *filetype_from_path(char *path);


#endif //ANTONENKO_OSISP_PRJ_2024_FILETYPE_H
