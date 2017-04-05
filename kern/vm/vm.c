#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <vm.h>
#include <addrspace.h>
#include <synch.h>

//static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

void
vm_bootstrap()
{
	IsInitialized = 1;
}


unsigned
int
coremap_used_bytes() {
        return usedpages*PAGE_SIZE;
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
        (void)ts;
        panic("vm tried to do tlb shootdown?!\n");
}

static
paddr_t
getppages(unsigned long npages)
{
/*        paddr_t addr;

        spinlock_acquire(&stealmem_lock);

        addr = ram_stealmem(npages);

        spinlock_release(&stealmem_lock);
        return addr;
*/

	unsigned long i=0;
        paddr_t paddr;
	for(;i<totalnumberofpages;i++)
        {       
                if(coremap[i].pagestate == FREE_STATE)
                        {
                                bool isValid = false;
                                if(i+npages<totalnumberofpages)
                                {
                                unsigned long  j=0;
                                for(;j<npages;j++)
                                {
                                if(coremap[i+j].pagestate != FREE_STATE)
                                        break;
                                }
                                if(j==npages)
                                        isValid = true;
                                }
                                if(isValid)
                                        break;
                        }
        }
        if(i==totalnumberofpages)
        {
	        return 0;
        }
	unsigned long begin = i;
        unsigned long end = begin + npages;
        for(;i<end;i++)
        {
        coremap[i].pagestate = FIXED_STATE;
        coremap[i].chunksize = 0;
        }
        coremap[begin].chunksize = npages;
        paddr = (begin+1)*PAGE_SIZE;
	usedpages += npages;
        return paddr;



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



/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(unsigned npages)
{
        paddr_t pa;

        vm_can_sleep();
        if(IsInitialized)
	{
	lock_acquire(coremaplock);
	pa = getppages(npages);
        lock_release(coremaplock);
	}
	else
	pa = getppages(npages);
	if (pa==0) {
                return 0;
        }
        return PADDR_TO_KVADDR(pa);
}


void
free_kpages(vaddr_t addr)
{       
        /* nothing - leak the memory. */
       paddr_t Kaddr = addr - MIPS_KSEG0;
       unsigned pageno = Kaddr/PAGE_SIZE-1;
	if(IsInitialized)
        {
	lock_acquire(coremaplock);

	unsigned no_pages = coremap[pageno].chunksize;		 
        for(unsigned long i=pageno;i<pageno+no_pages;i++)
	{
	coremap[i].pagestate = FREE_STATE;
	coremap[i].chunksize=0;
	}
	usedpages-=no_pages;
	lock_release(coremaplock);
	}
	else
	{
	unsigned no_pages = coremap[pageno].chunksize;
        for(unsigned long i=pageno;i<pageno+no_pages;i++)
        {
        coremap[i].pagestate = FREE_STATE;
        coremap[i].chunksize=0;
        }
        usedpages-=no_pages;
	}
}





int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	(void)faulttype;
	(void)faultaddress;
        return 0;
}
