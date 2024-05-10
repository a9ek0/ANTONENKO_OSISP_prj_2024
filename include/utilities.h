#ifndef ANTONENKO_OSISP_PRJ_2024_UTILITIES_H
#define ANTONENKO_OSISP_PRJ_2024_UTILITIES_H

#include "../include/filetype.h"

typedef struct filetype filetype;
typedef struct inode inode;

//void serialize_array(filetype *queue, int *front, int *rear, int *index);
void serialize_array(filetype *fs_root, int *index);
void deserialize_array(int *index, filetype *fs_root);
void deserialize_filetype_from_file(filetype *f, FILE *fp);
void serialize_filetype_to_file(filetype *f, FILE *fp);
void deserialize_inode_from_file(inode *i, FILE *fp);
void serialize_inode_to_file(inode *i, FILE *fp);

#endif //ANTONENKO_OSISP_PRJ_2024_UTILITIES_H
