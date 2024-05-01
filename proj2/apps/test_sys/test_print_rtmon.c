#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>

// Define the Syscall Number
#define SYS_set_rtmon 450
#define SYS_cancel_rtmon 451
#define SYS_print_rtmon 452

int main(int argc, char **argv)
{
//Check the no of arguments
	if (argc != 2) {
		printf("Incorrect Number of Arguments, expected 1\n");
		printf("pid\n");
		printf("Run: ./periodic 200 400 0 to get a PID\n");
		return -1;
	}


	pid_t pid = atoi(argv[1]);
	int ret = -999;

	printf("System call start: print_rtmon %i\n", pid);
	ret = syscall(SYS_print_rtmon, pid);
	printf("System  call returned %d.\n", ret);

	return ret;
}
