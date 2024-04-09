#ifndef SFS_H
#define SFS_H

#include "inode.h"
#include "superblock.h"
#include "filetype.h"
#include "operations.h"
#include "utilities.h"

void root_dir_init();
int save_contents();

#endif /* SFS_H */