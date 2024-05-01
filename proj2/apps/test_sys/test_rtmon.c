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

	if (argc != 4) {
		printf("Incorrect Number of Arguments, expected 3\n");
		printf("pid, C_ms, T_ms\n");
		printf("Run: ./periodic 200 400 0 to get a PID\n");
		return -1;
	}


	pid_t pid = atoi(argv[1]);
	unsigned int C_ms = atoi(argv[2]);
	unsigned int T_ms = atoi(argv[3]);
	int ret = -999;

//testing of set_rtmon
	printf("Starting system call: set_rtmon %i, %i, %i\n", pid, C_ms, T_ms);
	ret = syscall(SYS_set_rtmon, pid, C_ms, T_ms);
	printf("System call returned %d.\n", ret);

	
// Testing print_rtmon
	printf("Starting system call: print_rtmon %i\n", pid);
	ret = syscall(SYS_print_rtmon, pid);
	printf("System call returned %d.\n", ret);

	return ret;
}
