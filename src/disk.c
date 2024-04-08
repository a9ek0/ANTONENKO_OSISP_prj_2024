#include <stdbool.h>
#include <stdlib.h>

#define BLOCK_SIZE      (1<<12)
#define DISK_FAILURE    (-1)

typedef struct Disk Disk;

struct Disk {
    int	    fd;	      
    size_t  blocks;    
    size_t  reads;     
    size_t  writes;    
}; 

Disk *	disk_open();
void	disk_close();

ssize_t	disk_read();
ssize_t	disk_write();
