/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <current.h>
#include <mips/tlb.h>
#include <pagetable.h>
#include <synch.h>

//has to be changed
#define STACK_TEMP_VALUE    801
#define TEMP_VALUE    0
#define STACK_SIZE 4096
/*
static
void
as_zero_region(paddr_t paddr, unsigned npages)
{
        bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}

*/

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */


static void deletesegs(struct addrspace *as)
{
	struct segment* temp = as->segments;
        while(temp!=NULL)
        {
                struct segment* prev = temp;
                temp=temp->nextseg;
                kfree(prev);
        }

}

static void deletevandppages(struct addrspace *as)
{
	struct node* newtemp = as->head;
        //lock_acquire(as->ptlock);
	 while(newtemp!=NULL)
        {
		if(newtemp->ptentry==NULL)
		{
			kfree(newtemp);
                	break;
		}
		struct node* prev = newtemp;
                if(prev->ptentry->swapped)
		{
			deleteFromDisk(prev->ptentry->diskaddr);
		}
		else
		{
		unsigned pageno = getpageno(prev->ptentry->paddr);
		freeppages(pageno);
		}
                newtemp=newtemp->next;
		kfree(prev->ptentry);
                kfree(prev);
        }
	//lock_release(as->ptlock);

}
struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
        if (as==NULL) {
                return NULL;
        }
	as->segments = NULL;
	as->head = initializepagetable();        
	as->tail = as->head;
	as->ptlock = lock_create("as->ptlock");
	return as;

}

static
void
vm_can_sleep(void)
{
        if (CURCPU_EXISTS()) {
                /* must not hold spinlocks */
                KASSERT(curcpu->c_spinlocks == 0);

                /* must not be in an interrupt handler */
                KASSERT(curthread->t_in_interrupt == 0);
        }
}
int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new;

        vm_can_sleep();

        new = as_create();
        if (new==NULL) {
                return ENOMEM;
        }
        
	struct segment* temp = old->segments;
	new->segments = kmalloc(sizeof(struct segment));
        struct segment* newtemp = new->segments;
	newtemp->nextseg = NULL;
	if (newtemp==NULL) {
		//MEMLEAK???
//		as_destroy(new);
		return ENOMEM;
	}
	while(temp!=NULL)
		{
			newtemp->as_vbase = temp->as_vbase;
			newtemp->as_npages = temp->as_npages;
			temp=temp->nextseg;
			if(temp!=NULL)
			{
				newtemp->nextseg = kmalloc(sizeof(struct segment));
				if (newtemp->nextseg==NULL) {
         		       //MEMLEAK???
				deletesegs(new);		
//				as_destroy(new);
                		return ENOMEM; 
        			}
				newtemp->nextseg->nextseg=NULL;
			newtemp = newtemp->nextseg;
			}	
		}

		new->cur_stackpages = old->cur_stackpages;
		new->as_heapvbase = old->as_heapvbase;
		new->cur_heappages = old->cur_heappages;


	struct node* tempnode = old->head;
	struct node* newtempnode = new->head;
//	lock_acquire(old->ptlock);
	while(tempnode!=old->tail)
	{
		paddr_t paddr = getppages(1);
		 if (paddr == 0)
			{
			deletesegs(new); 
			deletevandppages(new);
//		lock_release(old->ptlock);
//			as_destroy(new);
                        return ENOMEM;
			}
		new->tail = addpagetableentries(tempnode->ptentry->vaddr,paddr,new->tail,new);
		if(new->tail==NULL)
			{
			deletesegs(new);
                        deletevandppages(new);
//		lock_release(old->ptlock);
//			as_destroy(new);
			return ENOMEM;
			}

		lock_acquire(old->ptlock);
		if(tempnode->ptentry->swapped)
		{
			lock_release(old->ptlock);
			if(swapin(tempnode->ptentry,old))
				{
					deletesegs(new);
                        		deletevandppages(new);
//		lock_release(old->ptlock);
	                 		return ENOMEM;
				}
		memmove((void *)PADDR_TO_KVADDR(newtempnode->ptentry->paddr),
                (const void *)PADDR_TO_KVADDR(tempnode->ptentry->paddr),
                PAGE_SIZE);
		stabilizenewallocation(tempnode->ptentry,curthread->t_proc->PID);
		}
		else
		{
		memmove((void *)PADDR_TO_KVADDR(newtempnode->ptentry->paddr),
                (const void *)PADDR_TO_KVADDR(tempnode->ptentry->paddr),
                PAGE_SIZE);
		lock_release(old->ptlock);
		}

		tempnode = tempnode->next;
		newtempnode = newtempnode->next;
	}
//		lock_release(old->ptlock);

        *ret = new;
        return 0;

}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	vm_can_sleep();
/*	struct segment* temp = as->segments;
        while(temp!=NULL)
        {
		struct segment* prev = temp;
                temp=temp->nextseg;
        	kfree(prev);
	}
*/
	deletesegs(as);
/*
	struct node* newtemp = as->head;
	while(newtemp!=NULL&&newtemp->ptentry!=NULL)
	{
		struct node* prev = newtemp;
		unsigned pageno = getpageno(prev->ptentry->paddr);
		freeppages(pageno);
		newtemp=newtemp->next;
		kfree(prev);
	}*/
	lock_acquire(as->ptlock);
	deletevandppages(as);
	lock_release(as->ptlock);
	lock_destroy(as->ptlock);
	kfree(as);
}

void
as_activate(void)
{
	int i, spl;
        struct addrspace *as;
        
        as = proc_getas();
        if (as == NULL) {
                return;
        }
        
        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();
        
        for (i=0; i<NUM_TLB; i++) {
                tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }
        
        splx(spl);

}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}



/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
	size_t npages;
        
        vm_can_sleep();
        
        /* Align the region. First, the base... */
        memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;
        
        /* ...and now the length. */
        memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;
        
        npages = memsize / PAGE_SIZE;
	
	(void)readable;
	(void)writeable;
	(void)executable;
	

	struct segment* temp = as->segments;
	struct segment* prev = temp;
	while(temp!=NULL)
	{
		prev = temp;
		temp=temp->nextseg;	
	}
	if(prev==NULL)
	{
	as->segments = kmalloc(sizeof(struct segment));
	as->segments->as_vbase = vaddr;
	as->segments->as_npages = npages;
	as->segments->nextseg=NULL;
	}
	else
	{
	prev->nextseg = kmalloc(sizeof(struct segment));
	prev->nextseg->as_vbase = vaddr;
        prev->nextseg->as_npages = npages;
	prev->nextseg->nextseg = NULL;
	}
	return 0;

}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	struct segment* temp = as->segments;
        struct segment* prev = temp;
	
	while(temp!=NULL)
        {
        	vm_can_sleep();
//		kprintf("\n%lu\n%lu\n%d\n",usedpages,totalnumberofpages,temp->as_npages);
//        	paddr_t padd = getppages(temp->as_npages);
        	//MEMLEAK??
//		if (padd == 0) 
//                	return ENOMEM;
//                as->tail = addpagetableentries(temp->as_vbase,padd,as->tail);
		if(prev->as_vbase<temp->as_vbase)
			prev = temp;
//		as_zero_region(padd, temp->as_npages);
		temp=temp->nextseg;
        }
/*        
	//Has to be changed
	paddr_t paddr = getppages(TEMP_VALUE);
        //MEMLEAK??
	if (paddr == 0) {
                return ENOMEM;
        }

	as->tail = addpagetableentries(USERSTACK-(STACK_SIZE*TEMP_VALUE),paddr,as->tail);
        as_zero_region(paddr, TEMP_VALUE);
	as->cur_stackpages = TEMP_VALUE;

	//Has to be changed
	int pageno = prev->as_vbase/PAGE_SIZE-1;
        int heappageno = pageno + prev->as_npages+1;
	as->as_heapvbase = heappageno*PAGE_SIZE;
	paddr = getppages(TEMP_VALUE);
	if (paddr == 0) {
                return ENOMEM;
        }
	
	as->tail = addpagetableentries(as->as_heapvbase,paddr,as->tail);
        as_zero_region(paddr, TEMP_VALUE);
	as->cur_heappages = TEMP_VALUE;
 
	(void)as;
  */
	as->cur_stackpages = STACK_TEMP_VALUE;
	int pageno = prev->as_vbase/PAGE_SIZE;
        int heappageno = pageno + prev->as_npages;
        as->as_heapvbase = heappageno*PAGE_SIZE;
	as->cur_heappages = TEMP_VALUE;
	return 0;

}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}

