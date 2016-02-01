#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
 
void
usage()
{
	printf("Usage: busylock <file> <offset> <length> <loop count>\n");
}

int
main(int argc, char* argv[])
{
	if (argc != 5) {
		usage();
		return -1;
	}

	char* filename = argv[1];
	int offset = atoi(argv[2]);
	int length = atoi(argv[3]);
	int loopCount  = atoi(argv[4]);
	if ((offset < 0) ||
	    (length <= 0) ||
	    (loopCount <= 0)) {
		usage();
		return -1;
	}
	printf("opening file: %s\n", filename);
	/* open for read/write */
	int fd = open(filename, O_RDWR);
	if (fd < 0) {
		usage();
		printf("filename not valid!\n");
		return -1;
	}

	struct flock lock, unlock;
	bzero(&lock, sizeof(lock));
	bzero(&unlock, sizeof(unlock));
	lock.l_type = F_WRLCK; /* write lock */
	lock.l_whence = SEEK_SET; /* start of file */
	lock.l_start = offset;
	lock.l_len = length;
	unlock.l_type = F_UNLCK; /* un lock */
	unlock.l_whence = SEEK_SET;
	unlock.l_start = offset;
	unlock.l_len = length;

	unsigned int i = 0;
	unsigned int s = 0;
	unsigned int f = 0;
	printf("testing byte range locks on file: %s\n", filename);
	while(i < loopCount) {
		/* F_SETLK to really take the lock */
		if (fcntl(fd, F_SETLK, &lock) < 0) {
			/* fail, most likely lock conflict */
			f++;
		} else { /* lock success */
			s++;
			/* F_SETLK to really unlock the lock */
			(void)fcntl(fd, F_SETLK, &unlock);
		} 
		i++;
	}

	printf("summary: total %d, pass %d, fail %d\n", loopCount, s, f);
	close(fd);
	return 0;
}

