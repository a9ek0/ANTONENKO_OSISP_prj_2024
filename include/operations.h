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
#include "fs_init.h"

# define UTIME_NOW	((1l << 30) - 1l)
# define UTIME_OMIT	((1l << 30) - 2l)

int sfs_mkdir(const char *path, mode_t mode);

int sfs_getattr(const char *path, struct stat *stat_buf);

int sfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);

int sfs_rmdir(const char *path);

int sfs_rm(const char *path);

int sfs_open(const char *path, struct fuse_file_info *fi);

int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int sfs_rename(const char *from, const char *to);

int sfs_utimens(const char *path, const struct timespec tv[2]);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static struct fuse_operations operations =
        {
                .mkdir=sfs_mkdir,
                .create=sfs_create,
                .rmdir=sfs_rmdir,
                .unlink=sfs_rm,
                .readdir=sfs_readdir,
                .getattr=sfs_getattr,
                .open=sfs_open,
                .write=sfs_write,
                .read=sfs_read,
                .rename=sfs_rename,
                .utimens=sfs_utimens,
        };
#pragma GCC diagnostic pop

#endif //ANTONENKO_OSISP_PRJ_2024_OPERATIONS_H
