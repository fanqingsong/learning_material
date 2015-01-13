#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "iomap.h"

#define BASE	0xe2000000

int main(int argc, char *argv[])
{
	int fd1 = open("/dev/iomap0", O_RDWR);
	int fd2 = open("/dev/iomap1", O_RDWR);
	Iomap dev1, dev2;

	if (fd1 == -1 || fd2 == -1) {
		perror("open");
		return 1;
	}

	/* setup first device */
	dev1.base = BASE;
	dev1.size = 1024 * 1024;
	if (ioctl(fd1, IOMAP_SET, &dev1)) {
		perror("ioctl");
		return 2;
	}

	/* setup second device, offset 1 meg from first */
	dev2.base = BASE + dev1.size;
	dev2.size = 1024 * 1024;
	if (ioctl(fd2, IOMAP_SET, &dev2)) {
		perror("ioctl");
		return 3;
	}
	
	return 0;
}
