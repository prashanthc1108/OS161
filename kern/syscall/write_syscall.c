#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>



int
sys___write(int fd, userptr_t buffer,size_t len)
{
int result;
	(void)fd;
	size_t i;
	char localbuffer[len];
	result = copyin(buffer,localbuffer,len);
	if(result==EFAULT)
		return EFAULT;
//	kprintf("%s",localbuffer);	
        for (i=0; i<len; i++) {
                putch(localbuffer[i]);
        }
	return result;

}
