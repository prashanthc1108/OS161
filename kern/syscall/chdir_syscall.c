#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>
#include <filetable.h>
#include <vfs.h>

int sys___chdir(userptr_t dirpath)
{
	size_t len =32; 
	size_t actual_len=0;
        char localbuffer[len];
	int result=copyinstr(dirpath, localbuffer, len, &actual_len);
        if(result==EFAULT)
                return EFAULT;
	char carray[actual_len];
	strcpy(carray,localbuffer);
	int ret = vfs_chdir(localbuffer);
	return ret;
}
