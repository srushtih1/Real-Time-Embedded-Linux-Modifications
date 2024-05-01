#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

static int __init module(void)
{
   printk(KERN_INFO "Loadable Kernel\n");

	printk(KERN_INFO "Hello world! team17 in kernel space!\n");
    return 0;
}

static void __exit cleanup(void)
{
    printk(KERN_INFO "Goodbye! You have been exited from the kernel\n");
}

module_init(module);
module_exit(cleanup);


