#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(int argc, char **argv){

	
	if (argc != 2) {
 		printf("Incorrect Number of Arguments, expected 1\n");
 		printf("Memory to allocate in bytes\n");
 		return -1;
 	}
	//Memory Size to be allocated Which is parse from the input
	size_t n = atoi(argv[1]);
	
	char *buf = malloc(n);
	if(mlock(buf, n)){  	//locking the memory here
		printf("./mem_alloc_lock: mlockall failed\n");
		return -1;
	}
	//checking the memory access time
	struct timespec T1, T2;
	long TotalT;
	clock_gettime(CLOCK_MONOTONIC, &T1);
	for (int i = 0; i <n ; i += 4096)
		buf[i] = 1;
	clock_gettime(CLOCK_MONOTONIC, &T2);
		
	//Time
	TotalT = T2.tv_nsec - T1.tv_nsec;
	
	//Print the performance
	pid_t pid = getpid();
	printf("PID %d, %ld ns\n",pid,TotalT);
	
	pause(); 

	//Unlocking the memory
	munlock(buf, n);
	free(buf);
}
