#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>


//define SYS_set_rtmon
#define SYS_set_rtmon 450
#define SYS_cancel_rtmon 451
#define SYS_print_rtmon 452

int main(int argc, char **argv)
{

	if (argc != 4) {
		printf("Number of Incorrect Arguments, expected 3\n");
		printf("pid, C_ms, T_ms\n");
		printf("Run: ./periodic 200 400 0 to get a PID\n");
		return -1;
	}

	
	pid_t pid = atoi(argv[1]);
	unsigned int C_ms = atoi(argv[2]);
	unsigned int T_ms = atoi(argv[3]);
	int ret = -999;

	// Test set_rtmon
	printf("system call start: set_rtmon %i, %i, %i\n", pid, C_ms, T_ms);
	ret = syscall(SYS_set_rtmon, pid, C_ms, T_ms);
	printf("System call returned %d.\n", ret);

	return ret;
}
