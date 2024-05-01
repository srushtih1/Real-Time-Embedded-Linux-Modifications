#include <time.h>
#include <sys/types.h>
 

int main(int argc, char **argv)
{
 
	if (argc != 2) {
 		printf("Incorrect Number of Arguments, expected 1\n");
 		printf("Memory to allocate in bytes\n");
 		return -1;
 	}
 

	size_t n = atoi(argv[1]);
 
 	char *buf = malloc(n); //allocate n bytes of memory
	


	struct timespec t1, t2;
	long totalT;

	clock_gettime(CLOCK_MONOTONIC, &t1);
	for (int i = 0; i < n; i += 4096)
 		buf[i] = 1;

	clock_gettime(CLOCK_MONOTONIC, &t2);

	totalT = t2.tv_nsec - t1.tv_nsec;
	printf("%zu, PID %d, %ld ns\n", n, getpid(), totalT);
 
 	//deallocate memory
 	free(buf);

	pause(); //wait until terminated with ctrl-c
 }
