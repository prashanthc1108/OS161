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

void sys___exit(int exitcode)
{
        lock_acquire(processtable->proclock);
        struct proc* nproc =  getprocessforPID(curthread->t_proc->PID);
        lock_release(processtable->proclock);
        if(nproc==NULL)
                return;
        V(nproc->pidsem);
	nproc->exitstatus = _MKWAIT_EXIT(exitcode);
	thread_exit();
        return;
}

