#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/kernel.h>

#define __NR_count_rt_tasks 449

int main(void) {
int answer;
long ret = syscall(__NR_count_rt_tasks, &answer);

if (ret == 0) {
printf("Number  of real-time tasks: %d\n", answer);
} else {
printf("System call failed with code %ld\n", ret);
}

return 0;
}





