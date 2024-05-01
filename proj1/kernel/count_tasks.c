#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/types.h>

asmlinkage long sys_count_rt_tasks(int *result) {
  struct task_struct *task;
  int count = 0;
  
  for_each_process(task) {
    if (task->prio < MAX_RT_PRIO)
      count++;
  }
  
  if (copy_to_user(result, &count, sizeof(int)))
    return -EFAULT;
  
  return 0;
}
SYSCALL_DEFINE1(count_rt_tasks, int __user *, result) {
  return sys_count_rt_tasks(result);
}
