#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


int my_cnt (char *filename) {
	int fd;
    char *data;
    struct stat sbuf;

	if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("open error.");
        exit(1);
    }

    if (stat(filename, &sbuf) == -1) {
        perror("stat error.");
        exit(1);
    }

	if ((data = mmap (0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (caddr_t) -1)
		err_sys ("mmap error.");

    int i=0, count=0;
    for (i=0; i<sbuf.st_size; i++) {
    	if (data[i] == '\n') {
    		count ++;
    	}
    }

	return count;
}