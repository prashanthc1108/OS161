#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>
#include <filetable.h>


int sys___close(int fd)
{
	if(fd<0||fd>63)
		return EBADF;
        int err = closefd(fd);
        return err;
}


