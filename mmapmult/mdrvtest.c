#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
 
#define PAGE_SIZE     4096 * 1
 
int main ( int argc, char **argv )
{
    int fd;
    char * address = NULL;
    char result[2*PAGE_SIZE]; 
    fd = open("/sys/kernel/debug/mdriver", O_RDWR);
    if(fd < 0) {
        perror("Open call failed");
        return -1;
    }
#if 1  
    address = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED)
    {
        perror("mmap operation failed");
        return -1;
    }
    memcpy(result, address, 20);
    printf("msg: %d\n", result[0]);
#endif
#if 0
    int ret = read(fd, result, 2*PAGE_SIZE);
    //printf("Initial message: %d\n", ret);
    //memcpy(address 11 , "*user*", 6);
    printf("msg: %c\n", result[0]);
#endif
    close(fd);
    munmap(address, PAGE_SIZE);
    return 0;
}
