#include <linux/syscalls.h>
#include <uapi/linux/sched/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h> 

#define MIN_TIME_MS 1
#define MAX_TIME_MS 10000
#define RTES_PRIORITY_LOW 50
#define RTES_PRIORITY_HIGH 70
#define RTES_NUM_RT_TASKS 20

struct rtes_task_info {
	struct list_head list; 
	pid_t rt_task_pid; 
	struct task_struct *tsk; 
};
LIST_HEAD(rtes_priority_list);

int map_realtime_priority(unsigned int val, unsigned int val_max,
			  unsigned int val_min)
{
	unsigned int new;


	if (val_max == val_min) {
		new = RTES_PRIORITY_HIGH;
	} else {
		new = RTES_PRIORITY_HIGH -
		      (RTES_PRIORITY_HIGH - RTES_PRIORITY_LOW) *
			      (val - val_min) / (val_max - val_min);
	}

	return new;
}

void update_realtime_priority(struct task_struct *tsk)
{
	unsigned int new_priority;
	unsigned int T_ms_min = MAX_TIME_MS  1;
	unsigned int T_ms_max = MIN_TIME_MS - 1;
	unsigned int T_ms;
	struct rtes_task_info *rtes_info;
	struct list_head *i; 
	struct list_head *temp; 
	struct sched_param new_sched_param;

	struct rtes_task_info *new_rtes_task_info =
		kmalloc(sizeof(struct rtes_task_info), GFP_KERNEL);
	new_rtes_task_info->rt_task_pid = tsk->pid;
	new_rtes_task_info->tsk = tsk;

	INIT_LIST_HEAD(&(new_rtes_task_info->list));
	list_add(&(new_rtes_task_info->list), &rtes_priority_list);

	list_for_each_safe (i, temp, &rtes_priority_list) {
		rtes_info = list_entry(i, struct rtes_task_info, list);
		T_ms = ktime_to_ns(rtes_info->tsk->T) / 1000000;

		if (rtes_info->tsk->state & TASK_DEAD) {
			list_del(&rtes_info->list);
			kfree(rtes_info);
			continue;
		}

		if (T_ms < T_ms_min) {
			T_ms_min = T_ms;
		}
		if (T_ms > T_ms_max) {
			T_ms_max = T_ms;
		}
	}


	list_for_each_safe (i, temp, &rtes_priority_list) {
		rtes_info = list_entry(i, struct rtes_task_info, list);
		T_ms = ktime_to_ns(rtes_info->tsk->T) / 1000000;
		// Calculate the RT priority
		new_priority = map_realtime_priority(T_ms, T_ms_max, T_ms_min);
		new_sched_param.sched_priority = new_priority;
		sched_setscheduler(rtes_info->tsk, SCHED_FIFO,
				   &new_sched_param);
		printk(KERN_INFO
		       "update_realtime_priority: pid = %i, new priority = %i\n",
		       rtes_info->rt_task_pid, new_priority);
	}
}

enum hrtimer_restart rtes_hrtimer_callback(struct hrtimer *timer)
{
	struct task_struct *tsk;
	pid_t pid;

	tsk = container_of(timer, struct task_struct, rtes_hr_timer);
	pid = tsk->pid;

	atomic_long_set(&(tsk->rtes_exec_time_ms), 0);
	tsk->start = ktime_get();
	wake_up_process(tsk);
	hrtimer_forward(timer, ktime_get(), tsk->T);
	return HRTIMER_RESTART;
}

SYSCALL_DEFINE3(set_rtmon, pid_t, pid, unsigned int, C_ms, unsigned int, T_ms)
{
	struct task_struct *p, *t;
	int ret = -1;

	// Validate the Arguments
	if (pid < 0) {
		return ret;
	}
	if (C_ms < MIN_TIME_MS || C_ms > MAX_TIME_MS) {
		return ret;
	}
	if (T_ms < MIN_TIME_MS || T_ms > MAX_TIME_MS) {
		return ret;
	}

	rcu_read_lock();
	for_each_process_thread (p, t) {
		if (t->pid == pid) {
			if (t->T != 0 && t->C != 0) {
				break;
			}
			// TODO(Piyush): Check for overflow
			t->C = ktime_set(0, C_ms * 1000000);
			t->T = ktime_set(0, T_ms * 1000000);
			atomic_long_set(&(t->rtes_exec_time_ms), 0);
			t->start = ktime_get();
			t->rtes_exec_last_ts = ktime_get();

			hrtimer_init(&(t->rtes_hr_timer), CLOCK_MONOTONIC,
				     HRTIMER_MODE_REL_PINNED);
			t->rtes_hr_timer.function = rtes_hrtimer_callback;

			update_realtime_priority(t);

			hrtimer_start(&(t->rtes_hr_timer), t->T,
				      HRTIMER_MODE_REL_PINNED);
			ret = 0;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

SYSCALL_DEFINE1(cancel_rtmon, pid_t, pid)
{
	unsigned int C_ms;
	unsigned int T_ms;
	struct task_struct *p, *t;
	struct rtes_task_info *rtes_info;
	struct list_head *i; 
	struct list_head *temp;

	int del_found = 0;
	int ret = -1;

	rcu_read_lock();
	for_each_process_thread (p, t) {
		if (t->pid == pid) {
			C_ms = ktime_to_ns(t->C) / 1000000;
			T_ms = ktime_to_ns(t->T) / 1000000;

			if (C_ms != 0 || T_ms != 0) {
				hrtimer_cancel(&(t->rtes_hr_timer));
				list_for_each_safe (i, temp,
						    &rtes_priority_list) {
					rtes_info = list_entry(
						i, struct rtes_task_info, list);
					if (rtes_info->rt_task_pid == pid) {
						del_found = 1;
						break;
					}
				}
				if (del_found == 1) {
					list_del(i);
				}
				t->C = ktime_set(0, 0);
				t->T = ktime_set(0, 0);
				ret = 0;
				break;
			}
		}
	}
	rcu_read_unlock();
	return ret;
}

SYSCALL_DEFINE1(print_rtmon, pid_t, pid)
{
	unsigned int C_ms;
	unsigned int T_ms;
	struct task_struct *p, *t;
	int ret = -1;

	rcu_read_lock();
	if (pid == -1) { 
		ret = 0;
		for_each_process_thread (p, t) {
			C_ms = ktime_to_ns(t->C) / 1000000;
			T_ms = ktime_to_ns(t->T) / 1000000;
			if (C_ms != 0 && T_ms != 0) {
				printk(KERN_INFO
				       "print_rtmon: PID %i, C %i ms, T %i ms\n",
				       t->pid, C_ms, T_ms);
			}
		}
	} else { 
		for_each_process_thread (p, t) {
			if (t->pid == pid) {
				C_ms = ktime_to_ns(t->C) / 1000000;
				T_ms = ktime_to_ns(t->T) / 1000000;
				printk(KERN_INFO
				       "print_rtmon: PID %i, C %i ms, T %i ms\n",
				       pid, C_ms, T_ms);
				ret = 0;
				break;
			}
		}
	}
	rcu_read_unlock();
	return ret;
}

SYSCALL_DEFINE0(wait_until_next_period)
{
	unsigned int C_ms;
	unsigned int T_ms;
	long A_ms;
	pid_t pid;

	struct task_struct *tsk = current;
	pid = tsk->pid;
	C_ms = ktime_to_ns(tsk->C) / 1000000;
	T_ms = ktime_to_ns(tsk->T) / 1000000;

	if (C_ms != 0 && T_ms != 0) {
		A_ms = (ktime_to_ns(ktime_get()) - ktime_to_ns(tsk->start)) /
		       1000000;
		if (A_ms > T_ms) {
			printk(KERN_ALERT
			       "PID %i: budget overrun (C %i ms, T %i, actual %ld ms)\n",
			       pid, C_ms, T_ms, A_ms);
		}
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		tsk->rtes_exec_last_ts = ktime_get();
		return 0;
	}

	return -1;
}
