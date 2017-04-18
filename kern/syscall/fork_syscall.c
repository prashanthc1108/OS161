#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>
#include <filetable.h>
#include <proc.h>
#include <proctable.h>
#include <thread.h>
#include <cpu.h>
#include <addrspace.h>
#include <current.h>
#include <syscall.h>
#include <mips/trapframe.h>

int sys___fork(struct trapframe *tf,int32_t* retval)
{
	struct proc* newproc = create_new_proc("childproc",retval);
	if(newproc==NULL)
		return *retval;
	*retval = as_copy(curthread->t_proc->p_addrspace, &newproc->p_addrspace);
	if(*retval)
		{
//		kprintf("\nhere1\n");
		proc_destroy(newproc);
		return *retval;
		}
	copyft(curthread->t_proc->ftab,newproc->ftab);
	struct trapframe* newtf = (struct trapframe*)kmalloc(sizeof(struct trapframe));
	if(newtf==NULL)
	{
//		kprintf("\nhere2\n");
		proc_destroy(newproc);
		return ENOMEM;
	}
	*newtf = *tf;

	*retval = thread_fork("childthread",newproc,enter_forked_process,newtf,0);
        if(*retval)
        {
//                kprintf("\nhere3\n");
		kfree(newtf);
                proc_destroy(newproc);
                return ENOMEM;
        }
	*retval = newproc->PID;
	return 0;
}
/*
void copyTF(struct trapframe *tf1,struct trapframe **tf2)
{
	struct trapframe* newtf = kmalloc(sizeof(struct trapframe*));
	newtf->tf_vaddr = tf1->tf_vaddr;      
        newtf->tf_status = tf1->tf_status;     
        newtf->tf_cause = tf1->tf_cause;     
        newtf->tf_lo = tf1->tf_lo;
        newtf->tf_hi = tf1->tf_hi;
        newtf->tf_ra = tf1->tf_ra;         
        newtf->tf_at = tf1->tf_at;         
        newtf->tf_v0 = tf1->tf_v0;         
        newtf->tf_v1 = tf1->tf_v1;        
        newtf->tf_a0 = tf1->tf_a0;
        newtf->tf_a1 = tf1->tf_a1;
        newtf->tf_a2 = tf1->tf_a2;
        newtf->tf_a3 = tf1->tf_a3;
        newtf->tf_t0 = tf1->tf_t0;
        newtf->tf_t1 = tf1->tf_t1;
        newtf->tf_t2 = tf1->tf_t2;
        newtf->tf_t3 = tf1->tf_t3;
        newtf->tf_t4 = tf1->tf_t4;
        newtf->tf_t5 = tf1->tf_t5;
        newtf->tf_t6 = tf1->tf_t6;
        newtf->tf_t7 = tf1->tf_t7;
        newtf->tf_s0 = tf1->tf_s0;
        newtf->tf_s1 = tf1->tf_s1;
        newtf->tf_s2 = tf1->tf_s2;
        newtf->tf_s3 = tf1->tf_s3;
        newtf->tf_s4 = tf1->tf_s4;
        newtf->tf_s5 = tf1->tf_s5;
        newtf->tf_s6 = tf1->tf_s6;
        newtf->tf_s7 = tf1->tf_s7;
        newtf->tf_t8 = tf1->tf_t8;
        newtf->tf_t9 = tf1->tf_t9;
        newtf->tf_gp = tf1->tf_gp;
        newtf->tf_sp = tf1->tf_sp;
        newtf->tf_s8 = tf1->tf_s8;
        newtf->tf_epc = tf1->tf_epc;        
	

	*tf2 = newtf;
}
*/
