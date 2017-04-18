#include <types.h>
#include <lib.h>
#include <pagetable.h>
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
struct node* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct node* tail)
{
	int pageno = getpageno(paddr);
        int chunksize = coremap[pageno].chunksize;
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* newtail = tail;
	for(int i=0;i<chunksize;i++)
	{
        	newtail->next = kmalloc(sizeof(struct node));
		if(newtail->next==NULL)
                        return NULL;
		newtail->next->ptentry = NULL;
		newtail->next->next =NULL;
		newtail->ptentry = kmalloc(sizeof(struct pte));
                if(newtail->ptentry==NULL)
                        return NULL;
		newtail->ptentry->vaddr = newvadd+i*PAGE_SIZE;
        	newtail->ptentry->paddr = paddr+i*PAGE_SIZE;
		newtail = newtail->next;
	}
	
	return newtail;
}

void deletepagetableentry(vaddr_t vaddr,struct node* head)
{
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* temp = head;
	struct node* prev = head;
	while(true)
	{
		if(temp->ptentry->vaddr!=newvadd)
		{
			prev = temp;	
			temp = temp->next;
			if(temp==NULL)
				return;
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
}

struct node* getpagetableentry(vaddr_t vaddr,struct node* head)
{
	vaddr_t newvadd = vaddr & PAGE_FRAME;
	struct node* temp = head;
	while(temp->ptentry!=NULL)
	{
			if(temp->ptentry->vaddr!=newvadd)
                	{	       
                        temp = temp->next;
                        if(temp==NULL)
                                return NULL;
                	}
                	else
                	{
                        	return temp;
                	}
	}
	return NULL;
}
