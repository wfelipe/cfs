#include <fcntl.h>
#include <stdio.h>

#define CFS_SUPER_MAGIC 0x19980122

int main (int argc, char **argv)
{
	char *file;
	int fd;
	char *buf;
	int ret;
	size_t blocksize = 4096;

	if (argc != 2) return 1;

	buf = malloc (blocksize);
	memset (buf, 0, blocksize);

	file = argv[argc-1];
	fd = open (file, O_RDWR);
	lseek (fd, blocksize, SEEK_SET);
	ret = read (fd, buf, 4096);
	printf ("%d .%s.\n", ret, buf);

	printf ("file %s\n", file);

	close (fd);

	return 0;
}
