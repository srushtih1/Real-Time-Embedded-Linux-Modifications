#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/mm_types.h>

#define MAX_INPUT_SIZE 20

static char input_buffer[MAX_INPUT_SIZE];

static ssize_t segment_info_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    pid_t pid;
    struct task_struct *task;
    struct mm_struct *mm;


    if (count > MAX_INPUT_SIZE) { printk(KERN_ALERT "segment_info: error: Failed : Input Size Larger Than 20\n");return -EINVAL;}
    if (copy_from_user(input_buffer, buf, count)) {    printk(KERN_ALERT "segment_info: error: Failed :  Copy Input From User space\n");return -EFAULT;}
    if (kstrtoint(input_buffer, 10, &pid)) {printk(KERN_ALERT "segment_info: error: failed to convert input to integer PID\n");return -EINVAL;}
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) { printk(KERN_ALERT "segment_info: error: Failed:  For this PID Task Struct Not Found%d\n", pid); return -EINVAL; }
    mm = task->mm;
    if (!mm) {  printk(KERN_ALERT "segment_info: error: no mm struct found for task with PID %d\n", pid); return -EINVAL; }
    printk(KERN_INFO "segment_info: [Memory segment addresses of process %d]\n", pid);
    printk(KERN_INFO "segment_info: %08lx - %08lx: code segment (%lu bytes)\n", mm->start_code, mm->end_code, mm->end_code - mm->start_code);
    printk(KERN_INFO "segment_info: %08lx - %08lx: data segment (%lu bytes)\n", mm->start_data, mm->end_data, mm->end_data - mm->start_data);

    return count;
}

static const struct file_operations segment_info_fops = {
    .owner = THIS_MODULE,
    .write = segment_info_write,
};

static struct miscdevice segment_info_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "segment_info",
    .fops = &segment_info_fops,
    .mode = S_IRWXUGO, 
};

static int __init segment_info_init(void)
{
    int ret;
    ret = misc_register(&segment_info_misc_device);
    if (ret) {  printk(KERN_ALERT "segment_info: error: Failed: register misc device\n"); return ret; }
    printk(KERN_INFO "segment_info: device node /dev/%s created\n", segment_info_misc_device.name);
    return 0;
}

static void __exit segment_info_exit(void)
{
    misc_deregister(&segment_info_misc_device);
    printk(KERN_INFO "segment_info: device node /dev/%s removed\n", segment_info_misc_device.name);
}

module_init(segment_info_init);
module_exit(segment_info_exit);

MODULE_LICENSE("GPL");

