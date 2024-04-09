#ifndef ANTONENKO_OSISP_PRJ_2024_OPERATIONS_H
#define ANTONENKO_OSISP_PRJ_2024_OPERATIONS_H

#define FUSE_USE_VERSION 30

#include "filetype.h"
#include "inode.h"
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "fuse.h"
#include "sfs.h"

int sfsmkdir(const char *path, mode_t mode);
int sfsgetattr(const char *path, struct stat *stat);
int sfsreaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi );

static struct fuse_operations operations =
        {
                .mkdir=sfsmkdir,
                .readdir=sfsreaddir,
                .getattr=sfsgetattr,
        };

#endif //ANTONENKO_OSISP_PRJ_2024_OPERATIONS_H
