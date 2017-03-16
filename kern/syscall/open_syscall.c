#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>
#include <filetable.h>
#include <limits.h>

int sys___open(userptr_t filename, int flags,int32_t * retval)
{
	size_t len =PATH_MAX; 
	size_t actual_len=0;
        char localbuffer[len];
/*	if(!(flags==0||flags==1||flags==2||flags==4||flags==8||flags==16||flags==32||flags==64))
		return EINVAL;
*/	int result=copyinstr(filename, localbuffer, len, &actual_len);
        if(result==EFAULT)
                return EFAULT;
/*	if(localbuffer==NULL)
		return ENOENT;
*/
//	char carray[actual_len];
//	strcpy(carray,localbuffer);
	(void)actual_len;
	int fd = getfd(localbuffer, flags, &result);
	if(fd == -1)
		return result;
	*retval = fd;
	return 0;
}

