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
#include <proctable.h>

int sys___waitPID(pid_t pid,userptr_t status,int options,int32_t* retval)
{

	if(options)
		return EINVAL;
	int result=0;
        int* dest = (int*)kmalloc(sizeof(int));
	*dest = 0;
	if(status!=NULL)
	{
        result = copyin(status,dest,sizeof(*dest));
        if(result==EFAULT)
		{
		kfree(dest);
                return EFAULT;
		}
	}
	lock_acquire(processtable->proclock);
	struct proc* nproc =  getprocessforPID(pid);
	lock_release(processtable->proclock);
	if(nproc==NULL)
		{
		kfree(dest);
		return ESRCH;
		}
	if(nproc->PPID!=curthread->t_proc->PID)
		{
		kfree(dest);
		return ECHILD;
		}
	P(nproc->pidsem);
	*retval = pid;
	*dest = nproc->exitstatus;
	if(status!=NULL)
	{
	result = copyout(dest, status, sizeof(*dest));
	if(result==EFAULT)
		{
		kfree(dest);
                return EFAULT;
		}
	}
	kfree(dest);
	lock_acquire(processtable->proclock);
	deleteprocess(pid);
	lock_release(processtable->proclock);
	return 0;
	
}
