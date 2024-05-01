#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define IOCTL_PRINT_HELLO _IO(0,0)
#define IOCTL_GET_TIME_NS _IO(0,1)
int main()
{
    int fd;
    long current_time;
    fd = open("/dev/device_driver", O_RDWR);
  
    ioctl(fd, IOCTL_PRINT_HELLO, 0);
    ioctl(fd, IOCTL_GET_TIME_NS, &current_time);
    printf("Current time in ns: %ld\n", current_time);  
    close(fd);
    
    return 0;
}





