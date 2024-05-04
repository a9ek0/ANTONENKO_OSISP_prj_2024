#ifndef SFS_H
#define SFS_H

#include "inode.h"
#include "superblock.h"
#include "filetype.h"
#include "operations.h"
#include "utilities.h"

#define MAX_FILES 31
#define INIT_DIRECTORIES 6
#define CHILDREN_PER_DIR 5

// Структура для хранения файловой системы
typedef struct {
    superblock s_block;
    filetype root;
    filetype file_array[MAX_FILES];
} filesystem_t;

extern filesystem_t fs;

void root_dir_init();

int save_contents();

int restore_contents(const char* filename);

void update_fs_structure(filesystem_t *fs, superblock *s_block, filetype *root_dir, filetype file_arr[]);

void apply_fs_structure(filesystem_t *fs);

int save_filetype(FILE *fp, const filetype *ft);

int load_filetypes(FILE *fp, filetype *ft);


#endif /* SFS_H */