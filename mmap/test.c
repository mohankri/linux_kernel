#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
 
#define PAGE_SIZE     4096
 
int main ( int argc, char **argv )
{
    int fd;
    char * address = NULL;
    char result[60]; 
    fd = open("/sys/kernel/debug/mdriver", O_RDWR);
    if(fd < 0) {
        perror("Open call failed");
        return -1;
    }
     
    address = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED)
    {
        perror("mmap operation failed");
        return -1;
    }
    memcpy(result, address, 50); 
    printf("Initial message: %s\n", result);
    //memcpy(address 11 , "*user*", 6);
    //printf("Changed message: %s\n", address);
    close(fd);
    return 0;
}
