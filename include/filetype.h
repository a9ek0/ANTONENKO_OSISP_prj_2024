#ifndef ANTONENKO_OSISP_PRJ_2024_FILETYPE_H
#define ANTONENKO_OSISP_PRJ_2024_FILETYPE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inode.h"

typedef struct inode inode;

typedef struct filetype {
    int valid;                   // Flag indicating if the filetype is valid
    char test[10];               // Test field for demonstration purposes
    char path[100];              // Path of the filetype
    char name[100];              // Name of the filetype
    inode *inum;                 // Pointer to the inode associated with the filetype
    struct filetype **children;  // Array of pointers to child filetypes
    int num_children;            // Number of child filetypes
    int num_links;               // Number of links to the filetype
    struct filetype *parent;     // Pointer to the parent filetype
    char type[20];               // Type of the filetype
} filetype;

extern filetype *root;
extern filetype file_array[31];

filetype *filetype_from_path(const char *path);

#endif //ANTONENKO_OSISP_PRJ_2024_FILETYPE_H
