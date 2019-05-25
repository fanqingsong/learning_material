#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "schar.h"

int main(int argc, char *argv[])
{
	int fd = open("/dev/schar", O_RDWR);

	/* complain if the open failed */
	if (fd == -1) {
		perror("open");
		return 1;
	}

	/* complain if the ioctl call failed */
	if (ioctl(fd, SCHAR_TOGGLE_DEBUG) == -1) {
		perror("ioctl");
		return 2;
	}

	printf("Schar debug toggled\n");

	return 0;
}

