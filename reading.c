#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int my_cnt (char *filename) {
	int i=0, count=0, j=0;
	FILE *fp;
	int chunkSize = getpagesize();
    char *data = malloc((chunkSize + 1) * sizeof(char));
    struct stat sbuf;

    if (stat(filename, &sbuf) == -1) {
        perror("stat error.");
        exit(1);
    }

    fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Error opening file");
		return(-1);
	}

	// round up pages
	int pages = (sbuf.st_size + chunkSize - 1) / chunkSize;
	for (j=0; j< pages; j++) {
		fseek(fp, SEEK_SET, j*chunkSize);
		int bytesRead = fread(data, chunkSize, 1, fp);

		if (bytesRead != chunkSize && j != pages - 1) {
			perror("Error reading file");
		}

    	for (i=0; i< strlen(data); i++) {
	    	if (data[i] == '\n') {
	    		count ++;
	    	}
	    }
    }

	fclose(fp);
	return count;
}