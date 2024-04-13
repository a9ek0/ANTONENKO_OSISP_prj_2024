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

int sfs_mkdir(const char *path, mode_t mode);
int sfs_getattr(const char *path, struct stat *stat);
int sfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi );
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int sfs_rmdir(const char * path);
int sfs_rm(const char * path);
int sfs_open(const char *path, struct fuse_file_info *fi);
int sfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi);
int sfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sfs_access(const char * path, int mask);
int sfs_rename(const char* from, const char* to);
int sfs_truncate(const char *path, off_t size);


static struct fuse_operations operations =
        {
                .mkdir=sfs_mkdir,
                .readdir=sfs_readdir,
                .getattr=sfs_getattr,
                .create=sfs_create,
                .rmdir=sfs_rmdir,
                .open=sfs_open,
                .read=sfs_read,
                .write=sfs_write,
                .rename=sfs_rename,
                .unlink=sfs_rm,
                /*
                 */
        };

#endif //ANTONENKO_OSISP_PRJ_2024_OPERATIONS_H
