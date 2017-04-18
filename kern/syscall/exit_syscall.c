#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <uio.h>
#include <filetable.h>
#include <current.h>
#include <proc.h>
#include <proctable.h>
#include <kern/wait.h>
#include <addrspace.h>

void sys___exit(int exitcode)
{
        lock_acquire(processtable->proclock);
        struct proc* nproc =  getprocessforPID(curthread->t_proc->PID);
        lock_release(processtable->proclock);
        if(nproc==NULL)
                return;
/*	
	if(nproc->PPID==2)
	{
		thread_exit();
		proc_destroy(nproc);
	}
	else
	{*/
	nproc->exitstatus = _MKWAIT_EXIT(exitcode);
        proc_remthread(curthread);
	 V(nproc->pidsem);
	thread_exit();
	
//	}
        return;
}

