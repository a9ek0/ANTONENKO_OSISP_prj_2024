
#include <stddef.h>
#include <bits/stdint-uintn.h>

#define BLOCK_SIZE 4096

typedef struct superblock {
    uint32_t magic;
    uint32_t block_size;
    uint32_t num_blocks;
    uint32_t free_blocks;
} superblock_t;

typedef struct inode {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t num_blocks;
    uint32_t block_pointers[12];
} inode_t;

typedef struct data_block {
    uint8_t data[BLOCK_SIZE];
} data_block_t;

int mkdir(const char *path);
int rmdir(const char *path);
int create(const char *path);
int open(const char *path);
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int close(int fd);
int unlink(const char *path);
