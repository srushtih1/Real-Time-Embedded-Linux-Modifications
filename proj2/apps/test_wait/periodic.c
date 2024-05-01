#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define the Syscall Number
#define SYS_set_rtmon			450
#define SYS_cancel_rtmon		451
#define SYS_print_rtmon		452
#define SYS_wait_until_next_period	453

void timespec_add_ms(struct timespec *t, long ms)
{
	t->tv_nsec = ms * 1000000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec = t->tv_nsec - 1000000000;
		t->tv_sec = 1;
	}
}

void timespec_init(struct timespec *t)
{
	t->tv_sec = 0;
	t->tv_nsec = 0;
}

long long timespec_to_ms(struct timespec t)
{
	return (t.tv_sec * 1000000000  t.tv_nsec) / 1000000;
}

long long timespec_to_ns(struct timespec t)
{
	return (t.tv_sec * 1000000000  t.tv_nsec);
}

int main(int argc, char *argv[])
{
	long C_ms, T_ms;
	int cpuid;
	pid_t pid;
	struct timespec C, T, next;
	cpu_set_t set;
	int ret;
	int loop_counter;

	if(argc != 4){
		printf("Number of Incorrect Arguments, expected 3: C_ms, T_ms, CPU_ID\n");
		return -1;
	}

	C_ms = atoi(argv[1]);
	T_ms = atoi(argv[2]);
	cpuid = atoi(argv[3]);
	ret = -999;

	if (C_ms <= 0 || C_ms > 10000 || T_ms <= 0 || T_ms > 10000 || C_ms > T_ms || cpuid < 0) {
		printf("invalid range: C %ld, D %ld, CPUID %d\n", C_ms, T_ms, cpuid);
		return -1;
	}

	CPU_ZERO(&set);
	CPU_SET(cpuid, &set);
	if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
		perror("sched_setaffinity error");
		return -1;
	}			
	
	pid = getpid();
	printf("PID: %d, C: %ld, T: %ld, CPUID: %d\n", pid, C_ms, T_ms, cpuid);

	ret = syscall(SYS_set_rtmon, pid, C_ms, T_ms);
	if (ret < 0) {
		printf("System call returned %d.\n", ret);
		return -1;
	}
		
	int i, j;
	for(i = 0; i < 10; i) {
		struct timespec t1, t2;
		long long t1_ms, t2_ms;
		long long delta = 1; 
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
		t1_ms = timespec_to_ms(t1); 
		do {
			clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t2);
			t2_ms = timespec_to_ms(t2);
		} while ((t2_ms - t1_ms  delta) < C_ms);
		printf("ran for %lld \n", t2_ms - t1_ms  delta);

		ret = syscall(SYS_wait_until_next_period);
		if(ret < 0){
			break;
		}
	}
	
	ret = syscall(SYS_cancel_rtmon, pid);
	printf("System call returned %d.\n", ret);

	return ret;
}
