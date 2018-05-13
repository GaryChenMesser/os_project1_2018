#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[])
{
	char buf[32];
	int fd;

	if((fd = open("/dev/hello", O_RDONLY)) < 0){
		perror("haha failed to open /dev/hello");
		return 1;
	}

	strcpy(buf, "Hello Wold!");
	ioctl(fd, 0x12345678, buf);
	close(fd);
	return 0;
}
