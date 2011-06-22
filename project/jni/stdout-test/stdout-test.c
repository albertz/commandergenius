#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern int main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	// Numerous Telechips devices will crash this simple Posix-compatibility test
	printf("printf()\n");
	fprintf(stdout, "fprintf(stdout)\n");
	fflush(stdout);
	fprintf(stderr, "fprintf(stderr)\n");
	fflush(stderr);
	write(STDOUT_FILENO, "write(1)\n", strlen("write(1)\n"));
	write(STDERR_FILENO, "write(2)\n", strlen("write(2)\n"));
	return 42;
}
