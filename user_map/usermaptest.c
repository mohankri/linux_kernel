#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define	DEV_NAME	"/dev/usermap"

int
main()
{
	int fd;
	char *ptr;
	
	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		perror("open");
		return (-1);
	}
	posix_memalign((void **)&ptr, 4096, 4096);
	memcpy(ptr, "Data from UserSpace", strlen("Data from UserSpace"));
	write(fd, ptr, 4096);
	printf("%s \n", ptr);
	close(fd);
}
