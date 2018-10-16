#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE     4096
//#define	ORDER	      10 //4MB
#define	ORDER	      1 //0=4K, 1=8K
#define	NUM_BUF	      1 
#define TOTAL 	      PAGE_SIZE * (NUM_BUF * (1 << ORDER))

int main ( int argc, char **argv )
{
    int fd;
    char * address = NULL;
    //char result[2*PAGE_SIZE]; 
    fd = open("/sys/kernel/debug/mdriver", O_RDWR);
    if(fd < 0) {
        perror("Open call failed");
        return -1;
    }
#if 1  
    printf("Total %d\n", TOTAL);
    address = mmap(NULL, TOTAL, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED)
    {
        perror("mmap operation failed");
        return -1;
    }
    //memcpy(result, address, 2*PAGE_SIZE);
    printf("msg: %c\n", address[0]);
    printf("msg: %c\n", address[1]);
    printf("msg: %c\n", address[2]);

   printf("msg: %c\n", address[4095]);
   printf("msg: %c\n", address[8190]);
/*    printf("msg: %c\n", address[4097]);
    printf("msg: %c\n", address[4098]); 

    printf("End msg: %c\n", address[8090]);
    printf("End msg: %c\n", address[8091]); */
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
