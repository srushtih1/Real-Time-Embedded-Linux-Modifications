#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/mm_types.h>


int pid_from_user;
static ssize_t write_operator(struct file *filp, const char __user * ubuf, size_t count, loff_t *f_pos){
	
	
	char kbuf[20]="";
	const char *p;
	
	struct task_struct* rtes_task;
	struct mm_struct* rtes_mm;
	struct vm_area_struct *rtes_vm_struct;
	
	struct vm_area_struct *rtes_vm_curr;
	unsigned long vm_address;
	int loaded_in_mem_count = 0;
	
	pte_t *rtes_ptep;
	spinlock_t *rtes_ptl;
	if(count>20){
		printk("vm_areas: error: Failed: Input Length Longer than 20 char\n");
		return -1;
	}
	
	if(copy_from_user(kbuf, ubuf, count)){
		printk("vm_areas: Failed\n");
		return -1;
	}
	
	p = strstrip(kbuf);
	
	if(kstrtoint(p, 0, &pid_from_user)){
		printk("vm_areas: error : Failed: Unable to Convert From String To INT");
		return -1;
	}
	rtes_task = pid_task(find_vpid(pid_from_user), PIDTYPE_PID);
	if(rtes_task==NULL){
		printk("vm_areas: error : Failed: Invalid PID");
		return -1;
	}
	rtes_mm = rtes_task->active_mm;
	if(rtes_mm){
		rtes_vm_struct = rtes_mm->mmap;
		rtes_vm_curr = rtes_vm_struct;
		printk("[Memory-mapped areas of process %d]\n", pid_from_user);
		do{
			loaded_in_mem_count=0;
			for(vm_address = rtes_vm_curr->vm_start; vm_address < rtes_vm_curr->vm_end; vm_address += 0x1000){
				if(follow_pte(rtes_mm, vm_address, &rtes_ptep, &rtes_ptl)){
					continue;
				}else{
					loaded_in_mem_count++;
					pte_unmap_unlock(rtes_ptep, rtes_ptl);
				}
			}
			if(rtes_vm_curr->vm_flags & VM_LOCKED){
				printk("%lx-%lx: %lu bytes [L], %d pages in physical memory", rtes_vm_curr->vm_start, rtes_vm_curr->vm_end, rtes_vm_curr->vm_end - rtes_vm_curr->vm_start, loaded_in_mem_count);
			}
			else{
				printk("%lx-%lx: %lu bytes, %d pages in physical memory", rtes_vm_curr->vm_start, rtes_vm_curr->vm_end, rtes_vm_curr->vm_end - rtes_vm_curr->vm_start, loaded_in_mem_count);
			}
			
			rtes_vm_curr = rtes_vm_curr->vm_next;
		
		}
		while(rtes_vm_curr!=NULL);
		
		
	}else{
		printk("vm_areas: error\n");
	}
	

	
	return count;
}

int fn_close(struct inode *inodep, struct file *filp){

	printk("vm_areas: module unloaded\n");
	return 0;
}


static const struct file_operations fops = {
	.write = write_operator,
	.release = fn_close,
	
};

struct miscdevice vm_areas = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "vm_areas",
	.fops= &fops,
};

int vm_areas_device_init(void){
	int error;

	error = misc_register(&vm_areas);
	if(error){
		printk("vm_areas:error : Failed : misc_register failed \n");
		return error;
	}
	printk("vm_areas: registered\n");
	printk(KERN_INFO "vm_areas: device node /dev/%s created\n", vm_areas.name);
	
	return 0;
}

void vm_areas_device_exit(void){
	misc_deregister(&vm_areas);
	printk(KERN_INFO "vm_areas: device node /dev/%s removed\n", vm_areas.name);
	printk(KERN_INFO "vm_areas: deregistered\n");
}

module_init(vm_areas_device_init);
module_exit(vm_areas_device_exit);
MODULE_LICENSE("GPL");

