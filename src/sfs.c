#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <asm-generic/errno-base.h>
#include "sfs.h"

static superblock_t superblock;
static inode_t *inodes;
static data_block_t *data_blocks;

int mkdir(const char *path) {
    return 0;
}


int rmdir(const char *path) {
}

int create(const char *path) {
}

int open(const char *path) {
}

int read(int fd, void *buf, size_t count) {
}

int write(int fd, const void *buf, size_t count) {
}

int close(int fd) {
}

int unlink(const char *path) {
}
