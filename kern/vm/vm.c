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
#include <vnode.h>
#include <stat.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <bitmap.h>
#include <uio.h>
#include <proctable.h>

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
//static struct spinlock vm_lock = SPINLOCK_INITIALIZER;

void
vm_bootstrap()
{
	SWAPPINGENABLED = false;
	get_swap_bitmap();
}



void get_swap_bitmap(){
	disk_v_node = kmalloc(sizeof(struct vnode));
        off_t swapSize;
        struct stat metadata;
        unsigned numberOfSwapPages;
        char disk[32];
        strcpy(disk,"lhd0raw:");
        int ret = vfs_open(disk,O_RDWR,0664,&disk_v_node);
        if(ret == 0){
                SWAPPINGENABLED = true;
                VOP_STAT(disk_v_node,&metadata);
                swapSize = metadata.st_size;
                numberOfSwapPages = swapSize/PAGE_SIZE;
                if(swapSize%PAGE_SIZE)
                        numberOfSwapPages++;
                swap_bitmap = bitmap_create(numberOfSwapPages);
        	swap_lock = lock_create("swaplock");
		}
}

int blockread(unsigned slot, paddr_t dest)
{
	off_t disk_offset = slot*PAGE_SIZE;

        struct iovec iov;
        struct uio ku;

	lock_acquire(swap_lock);
	uio_kinit(&iov, &ku, (userptr_t)PADDR_TO_KVADDR(dest), PAGE_SIZE, disk_offset, UIO_READ);
        int result = VOP_READ(disk_v_node, &ku);
	bitmap_unmark(swap_bitmap,slot);
	lock_release(swap_lock);
	return result;

}

unsigned blockwrite(paddr_t src,unsigned* freeLocOnDisk){
	lock_acquire(swap_lock);
	int ret = bitmap_alloc(swap_bitmap, freeLocOnDisk);
	off_t disk_offset = *freeLocOnDisk*PAGE_SIZE;
	if(ret!=0){
	//no space on disk, return appropriately
	lock_release(swap_lock);
	return ret;
	}
	else
	{
		struct iovec iov;
		struct uio ku;

		uio_kinit(&iov, &ku, (userptr_t)PADDR_TO_KVADDR(src), PAGE_SIZE, disk_offset, UIO_WRITE);
		
		int result = VOP_WRITE(disk_v_node, &ku);
		//what to do with result?
		(void)result;
		lock_release(swap_lock);
		
		return 0;
	}
}

void deleteFromDisk(unsigned slot){
	lock_acquire(swap_lock);
	bitmap_unmark(swap_bitmap,slot);
	lock_release(swap_lock);
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
//        panic("vm tried to do tlb shootdown?!\n");

	as_activate();
}

int LRU(unsigned long npages)
{
	spinlock_acquire(&stealmem_lock);
	unsigned long i;
	while(true)
	{
	i = LRUindex;
	for(;i<totalnumberofpages;i++)
		{
		if(coremap[i].pagestate==FIXED_STATE)
			continue;
		if(coremap[i].recentlyused)
			coremap[i].recentlyused = false;
		else
			{

			unsigned long j=i;
			unsigned long max=i+npages;
			for(;j<max;j++)
				coremap[j].pagestate=FIXED_STATE;
			LRUindex = i+1;
			if(LRUindex==totalnumberofpages)
				LRUindex = LRUstartindex;
			spinlock_release(&stealmem_lock);
			return i;
			}		 	
		}
	LRUindex=LRUstartindex;
	}	
	spinlock_release(&stealmem_lock);
	return 0;
}

paddr_t swapout(unsigned long npages)
{
	int pageno = LRU(npages);
	for(unsigned long i=0;i<npages;i++)
		{
		struct proc* process = getprocessforPID(coremap[pageno+i].PID);
			if(process!=NULL)
			{
			struct addrspace* addr = process->p_addrspace; 
			if(addr!=NULL&&addr->head!=NULL)
				{
				lock_acquire(addr->ptlock);
				clearTLB();		
				struct pte* ptentry =  getpagetableentrywithpaddr((pageno+i)*PAGE_SIZE,addr->head,addr)->ptentry;
				unsigned freeLocOnDisk;
				unsigned index = blockwrite((pageno+i)*PAGE_SIZE,&freeLocOnDisk);
		
				if(index==ENOSPC)
					{
					lock_release(addr->ptlock);
					return 0;
					}
				ptentry->paddr = 0;
				ptentry->diskaddr = freeLocOnDisk;
				ptentry->swapped = true;
				lock_release(addr->ptlock);
				}
			}
		}
	return (pageno)*PAGE_SIZE;
}


int swapin(struct pte* ptentry,struct addrspace* addr)
{
	paddr_t paddr = getppages(1);
	if(paddr==0)
	{
		return ENOMEM;
	}
	lock_acquire(addr->ptlock);
	blockread(ptentry->diskaddr,paddr);
	ptentry->swapped=false;
	ptentry->paddr = paddr;
	lock_release(addr->ptlock);
	return 0;	
}

static paddr_t getppagesifpresent(unsigned long npages)
{
	spinlock_acquire(&stealmem_lock);
        unsigned long i=0;
        paddr_t paddr;
        for(;i<totalnumberofpages;i++)
        {
                if(coremap[i].pagestate == FREE_STATE)
                        {
                                bool isValid = false;
                                if(i+npages<=totalnumberofpages)
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
        paddr = (begin)*PAGE_SIZE;
        usedpages += npages;
        spinlock_release(&stealmem_lock);
        return paddr;


}

paddr_t
getppages(unsigned long npages)
{
	paddr_t paddr;
	paddr = getppagesifpresent(npages);
//	kprintf("%lu\n",(unsigned long int)paddr);
	if(paddr==0)
	{
	if(curthread->t_proc->PID==2||!SWAPPINGENABLED)
                {
                return 0;
                }
	paddr = swapout(npages);
	}
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
	return paddr/PAGE_SIZE;
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

void stabilizenewallocation(struct pte* ptentry,int PID)
{
	spinlock_acquire(&stealmem_lock);
                unsigned long pageno = getpageno(ptentry->paddr);
                coremap[pageno].pagestate = DIRTY_STATE;
                coremap[pageno].PID = PID;
                coremap[pageno].recentlyused = true;
                spinlock_release(&stealmem_lock);
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
		{
			success = true;
		}
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
	lock_acquire(as->ptlock);	
	struct node* listentry = getpagetableentry(faultaddress,as->head,as);
	if(listentry==NULL)
	{
		lock_release(as->ptlock);
		paddr = getppages(1);
		if (paddr == 0) 
                        return ENOMEM;
		as->tail = addpagetableentries(faultaddress,paddr,as->tail,as);
		as_zero_region(paddr, 1);
		spinlock_acquire(&stealmem_lock);
		unsigned long pageno = getpageno(paddr);
		coremap[pageno].pagestate = DIRTY_STATE;
		coremap[pageno].PID = curthread->t_proc->PID;
		coremap[pageno].recentlyused = true;
		spinlock_release(&stealmem_lock);	
	}
	else
	{
		if(listentry->ptentry->swapped)
		{
			lock_release(as->ptlock);
			if(swapin(listentry->ptentry,as))
			{
			return ENOMEM;
			}
		paddr = listentry->ptentry->paddr;
		stabilizenewallocation(listentry->ptentry,curthread->t_proc->PID);
		}
		else
		{
			paddr = listentry->ptentry->paddr;
			lock_release(as->ptlock);
		}
	}
//	lock_release(as->ptlock);
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
