#include <types.h>
#include <lib.h>
#include <pagetable.h>
#include <synch.h>

/*
struct ptable* initializepagetable()
{
	struct ptable* pagetable = kmalloc(sizeof(struct ptable));
	return pagetable;	
}


//get the top 10 bits and the 2nd 10 bits from VA. top 10 bits is the index into the first level.2nd 10bits are index into 2nd level
//create a pagetableentry
//Get the physical page from core map from which extract the chunksize and then and the remaining pagetableentries 

struct pte* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct ptable* pagetable)
{
	vaddr_t leveloneindex = (vaddr&LEVEL_ONE_MASK)>>22;
	vaddr_t leveltwoindex = (vaddr&LEVEL_TWO_MASK)>>12;
	if(pagetable->firstlevel[leveloneindex]==NULL)
	{
		pagetable->firstlevel[leveloneindex] = kmalloc(sizeof(struct ltable));
	}
	struct pte* newpte = kmalloc(sizeof(struct pte));
	pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex] = newpte;
	newpte->paddress = paddr;
	int pageno = paddr/PAGE_SIZE -1;
	struct coremapentry * cmape = coremap[pageno];
	int chunksize = cmape->chunksize;
	if(chunksize>1)
	{
		for (int i=1;i<chunksize;i++)
		{
			vaddr_t tempaddr = vaddr+i*PAGE_SIZE;
			leveloneindex = (tempaddr&LEVEL_ONE_MASK)>>22;
        		leveltwoindex = (tempaddr&LEVEL_TWO_MASK)>>12;
        		if(pagetable->firstlevel[leveloneindex]==NULL)
        		{       
                		pagetable->firstlevel[leveloneindex] = kmalloc(sizeof(struct ltable));
        		}       
        		struct pte* temppte = kmalloc(sizeof(struct pte));
        		pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex] = temppte;
        		newpte->paddress = paddr;
		}
	}
}

void deletepagetableentry(vaddr_t vaddr)
{
	vaddr_t leveloneindex = (vaddr&LEVEL_ONE_MASK)>>22;
        vaddr_t leveltwoindex = (vaddr&LEVEL_TWO_MASK)>>12;
        if(pagetable->firstlevel[leveloneindex]==NULL)
        {
		return;       
        }       
	if(pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex]==NULL)
	{
		return;
	}
	kfree(pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex]);
	kfree(pagetable->firstlevel[leveloneindex]);
	//Not sure if necessary but no harm done
	pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex] = NULL:
	pagetable->firstlevel[leveloneindex] = NULL;
}

struct pte* getpagetableentry(vaddr_t vaddr)
{
	vaddr_t leveloneindex = (vaddr&LEVEL_ONE_MASK)>>22;
        vaddr_t leveltwoindex = (vaddr&LEVEL_TWO_MASK)>>12;
        if(pagetable->firstlevel[leveloneindex]==NULL)
        {       
                return NULL;
        }
        if(pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex]==NULL)
        {
                return NULL;
        }
	
	return pagetable->firstlevel[leveloneindex]->pteaddress[leveltwoindex];
}
*/

struct node* initializepagetable()
{
	struct node* newpt = kmalloc(sizeof(struct node));
	newpt->ptentry = NULL;
	return newpt;
}	

//returns the new tail. returning tail so that addition is O(1)
//Have not included permission information in PTE
struct node* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct node* tail,struct addrspace* as)
{
	(void)as;
//	int pageno = getpageno(paddr);
//        int chunksize = coremap[pageno].chunksize;
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* newtail = tail;
//	lock_acquire(as->ptlock);
//	for(int i=0;i<chunksize;i++)
//	{
        	newtail->next = kmalloc(sizeof(struct node));
		if(newtail->next==NULL)
		{
			lock_release(as->ptlock);
                        return NULL;
		}
		newtail->next->ptentry = NULL;
		newtail->next->next =NULL;
		newtail->ptentry = kmalloc(sizeof(struct pte));
                if(newtail->ptentry==NULL)
		{
			lock_release(as->ptlock);
                        return NULL;
		}
//		newtail->ptentry->vaddr = newvadd+i*PAGE_SIZE;
//        	newtail->ptentry->paddr = paddr+i*PAGE_SIZE;
		newtail->ptentry->vaddr = newvadd;
                newtail->ptentry->paddr = paddr;
		newtail->ptentry->swapped = false;
		newtail = newtail->next;
//	}
//	lock_release(as->ptlock);
	return newtail;
}

void deletepagetableentry(vaddr_t vaddr,struct node* head,struct addrspace* as)
{
	(void)as;
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* temp = head;
	struct node* prev = head;
//	lock_acquire(as->ptlock);
	while(true)
	{
		if(temp->ptentry->vaddr!=newvadd)
		{
			prev = temp;	
			temp = temp->next;
			if(temp==NULL)
			{
				lock_release(as->ptlock);
				return;
			}
		}
		else
		{
			break;
		}
	}
	prev->next = temp->next;
	//MEMLEAK?
	kfree(temp->ptentry);
	kfree(temp);	
//	lock_release(as->ptlock);
}

struct node* getpagetableentry(vaddr_t vaddr,struct node* head,struct addrspace* as)
{
	(void)as;
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* temp = head;
//	lock_acquire(as->ptlock);
	while(temp->ptentry!=NULL)
	{
			if(temp->ptentry->vaddr!=newvadd)
                	{	       
                        temp = temp->next;
                        if(temp==NULL)
				{
//				lock_release(as->ptlock);
                                return NULL;
				}
                	}
                	else
                	{
//				lock_release(as->ptlock);
                        	return temp;
                	}
	}
//	lock_release(as->ptlock);
	return NULL;
}


struct node* getpagetableentrywithpaddr(paddr_t paddr,struct node* head,struct addrspace* as)
{
	(void)as;
        paddr_t newvadd = paddr & PAGE_FRAME;
        struct node* temp = head;
//	lock_acquire(as->ptlock);
        while(temp->ptentry!=NULL)
        {
                        if(temp->ptentry->paddr!=newvadd)
                        {
                        temp = temp->next;
                        if(temp==NULL)
			{
//                        	lock_release(as->ptlock);
			        return NULL;
                        
			}
			}
                        else
                        {
//				lock_release(as->ptlock);
                                return temp;
                        }
        }
//	lock_release(as->ptlock);
	(void)as;  
      return NULL;
}

