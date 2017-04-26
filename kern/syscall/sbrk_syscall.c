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
#include <pagetable.h>

int sys___sbrk(intptr_t amount,int32_t* retval)
{
	if(amount%PAGE_SIZE!=0)
		return EINVAL;
	struct addrspace* as = curthread->t_proc->p_addrspace;
	vaddr_t old = as->as_heapvbase+as->cur_heappages*PAGE_SIZE;
	int npages = amount/PAGE_SIZE;	
	int newnum = as->cur_heappages+npages;
	
	if(npages>0)
	{
/*		if(npages>=(int)(totalnumberofpages-usedpages))
		{
			*retval = -1;
			return ENOMEM;
		}
*/
		
	}
	else
	{
		if(newnum<0)
		{
			*retval = -1;
			return EINVAL;	
		}
		else
		{
			vaddr_t addr = as->as_heapvbase+newnum*PAGE_SIZE;
			for(int i=0;i<-npages;i++)
			{
				struct node* node = getpagetableentry(addr+i*PAGE_SIZE,as->head,as);
				if(node==NULL)
					continue;
				unsigned pageno = getpageno(node->ptentry->paddr);
                		freeppages(pageno);
				deletepagetableentry(addr+i*PAGE_SIZE,as->head,as);
			}		
		as_activate();
		}	
	}

	vaddr_t newaddr = as->as_heapvbase+newnum*PAGE_SIZE;
	if(newaddr>(USERSTACK - as->cur_stackpages * PAGE_SIZE))
		{
		*retval = -1;
		return ENOMEM;	
		}
	as->cur_heappages = newnum;
	*retval = old;
	return 0;
}

