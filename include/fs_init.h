#ifndef SFS_H
#define SFS_H

#include "../include/inode.h"
#include "../include/superblock.h"
#include "../include/filetype.h"
#include "../include/operations.h"
#include "../include/utilities.h"

#define MAX_FILES 31

void root_dir_init();

int save_system_state();

void restore_file_system();

#endif /* SFS_H */