#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <uio.h>
#include <vfs.h>
#include <filetable.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>
#include <limits.h>


int
sys____getcwd(userptr_t buffer,size_t len,int32_t* retval)
{
        int result = 0;
	char buf[PATH_MAX+1];
        struct iovec iov;   
        struct uio ku;

/*	should we copyin?
        size_t actual_len=0;
        char localbuffer[len];
        int result=copyinstr(buffer, localbuffer, len, &actual_len);
        if(result==EFAULT)
                return EFAULT;*/
	

	(void)len;
	(void)retval;
        uio_kinit(&iov, &ku, buf, sizeof(buf)-1, 0, UIO_READ);
        result = vfs_getcwd(&ku);

	if (result)
		return result;

	buf[sizeof(buf)-1-ku.uio_resid] = 0;
        result = copyout(buf, buffer, sizeof(buf));
        if (result) {
                return result;
        }

	
	return result;

}

