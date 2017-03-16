#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <uio.h>
#include <filetable.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>



int
sys___dup2(int ofd, int dfd,int32_t* retval)
{
	int result = 0;
        result =  isfdvalidforDUP2(ofd,1);
	if(result)
                return result;
        result =  isfdvalidforDUP2(dfd,0);
	if(result)
                return result;


        result = clonefd(ofd,dfd);
	*retval	= dfd;
	return result;

}

