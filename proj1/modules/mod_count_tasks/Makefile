KERNEL_DIR =  ../../..
PWD = $(shell pwd)

obj-m += mod_count_tasks.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
