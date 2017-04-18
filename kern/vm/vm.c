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
#include <pagetable.h>
#include <addrspace.h>
#include <synch.h>

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
//static struct spinlock vm_lock = SPINLOCK_INITIALIZER;

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

paddr_t
getppages(unsigned long npages)
{
/*        paddr_t addr;

        spinlock_acquire(&stealmem_lock);

        addr = ram_stealmem(npages);

        spinlock_release(&stealmem_lock);
        return addr;
*/
	spinlock_acquire(&stealmem_lock);
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
		spinlock_release(&stealmem_lock);
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
	spinlock_release(&stealmem_lock);
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
/*
        paddr_t pa;

        vm_can_sleep();
        if(IsInitialized)
	{
	spinlock_acquire(&stealmem_lock);
	//lock_acquire(coremaplock);
	pa = getppages(npages);
	spinlock_release(&stealmem_lock);
        //lock_release(coremaplock);
	}
	else
	pa = getppages(npages);
	if (pa==0) {
                return 0;
        }
        return PADDR_TO_KVADDR(pa);

*/
	paddr_t pa;
        vm_can_sleep();	
//	spinlock_acquire(&stealmem_lock);
	pa = getppages(npages);
        
//	spinlock_release(&stealmem_lock);
	if (pa==0) {
                return 0;
        }
        return PADDR_TO_KVADDR(pa);
}


void
free_kpages(vaddr_t addr)
{       
        /* nothing - leak the memory. */
/*       paddr_t Kaddr = addr - MIPS_KSEG0;
       unsigned pageno = Kaddr/PAGE_SIZE-1;
	if(IsInitialized)
        {
	//lock_acquire(coremaplock);
	spinlock_acquire(&stealmem_lock);
	unsigned no_pages = coremap[pageno].chunksize;		 
        for(unsigned long i=pageno;i<pageno+no_pages;i++)
	{
	coremap[i].pagestate = FREE_STATE;
	coremap[i].chunksize=0;
	}
	usedpages-=no_pages;
	//lock_release(coremaplock);
	spinlock_release(&stealmem_lock);
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
*/
	paddr_t Kaddr = addr - MIPS_KSEG0;
       unsigned pageno = getpageno(Kaddr);
//	spinlock_acquire(&stealmem_lock);
	freeppages(pageno);
 //       spinlock_release(&stealmem_lock);

}

unsigned getpageno(paddr_t paddr)
{
	return paddr/PAGE_SIZE-1;
}

void freeppages(unsigned pageno)
{
        spinlock_acquire(&stealmem_lock);
	unsigned no_pages = coremap[pageno].chunksize;           
        for(unsigned long i=pageno;i<pageno+no_pages;i++)
        {
        coremap[i].pagestate = FREE_STATE;
        coremap[i].chunksize=0;
        }
        usedpages-=no_pages;
	spinlock_release(&stealmem_lock);
        //lock_release(coremaplock);
}

static
void
as_zero_region(paddr_t paddr, unsigned npages)
{       
        bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}


int
vm_fault(int faulttype, vaddr_t faultaddress)
{
/*	(void)faulttype;
	(void)faultaddress;
        return 0;
*/
	vaddr_t stackbase,stacktop,heapbase,heaptop,vbase, vtop;
	paddr_t paddr;
        int i;
        uint32_t ehi, elo;
        struct addrspace *as;
        int spl;

        faultaddress &= PAGE_FRAME;


        switch (faulttype) {
            case VM_FAULT_READONLY:
            //have not implemented permissions yet
	    case VM_FAULT_READ:
            case VM_FAULT_WRITE:
                break;
            default:
                return EINVAL;
        }

        if (curproc == NULL) {
                /*
                 * No process. This is probably a kernel fault early
                 * in boot. Return EFAULT so as to panic instead of
                 * getting into an infinite faulting loop.
                 */
                return EFAULT;
        }

        as = proc_getas();
        if (as == NULL) {
                /*
                 * No address space set up. This is probably also a
                 * kernel fault early in boot.
	         */
                return EFAULT;
        }
	/*get the segment to which the VA belongs*/
	struct segment* temp = as->segments;
	bool success = false;
        while(temp!=NULL)
        {
		vbase = temp->as_vbase;
		
		vtop = vbase + temp->as_npages * PAGE_SIZE;		
                if (faultaddress >= vbase && faultaddress < vtop)
		{
			success = true;
			
		}
		temp=temp->nextseg;
        }

	if(success==false)
	{
        	stackbase = USERSTACK - as->cur_stackpages * PAGE_SIZE;
        	stacktop = USERSTACK;
		if (faultaddress >= stackbase && faultaddress < stacktop)
			success = true;
	}
	if(success == false)
	{
		
		heapbase = as->as_heapvbase;
		heaptop = heapbase + as->cur_heappages * PAGE_SIZE;
		if (faultaddress >= heapbase && faultaddress < heaptop)
                        success = true;
		else
			return EFAULT;
	}
        
	
	struct node* listentry = getpagetableentry(faultaddress,as->head);
	if(listentry==NULL)
	{
	//	spinlock_acquire(&stealmem_lock);
		paddr = getppages(1);
	  //      spinlock_release(&stealmem_lock);
		if (paddr == 0) 
                        return ENOMEM;
		as->tail = addpagetableentries(faultaddress,paddr,as->tail);
//		kprintf("\n%lu\n",usedpages);
		as_zero_region(paddr, 1);
	}
	else
	{	
	paddr = listentry->ptentry->paddr;
	}
	/* make sure it's page-aligned */
        KASSERT((paddr & PAGE_FRAME) == paddr);

        /* Disable interrupts on this CPU while frobbing the TLB. */

        spl = splhigh();
//	spinlock_acquire(&vm_lock);

//	as_activate();
        for (i=0; i<NUM_TLB; i++) {
                tlb_read(&ehi, &elo, i);
                if (elo & TLBLO_VALID) {
		continue;
                }
                ehi = faultaddress;
                elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
                DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
                tlb_write(ehi, elo, i);
  //      	spinlock_release(&vm_lock);	
                splx(spl);
                return 0;
        }
	        as_activate();
		ehi = faultaddress;
                elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
                DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		tlb_write(ehi, elo, 0);
    //    	spinlock_release(&vm_lock);	
                splx(spl);
                return 0;       
	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
        splx(spl);
        return EFAULT;

}
