#include <vm.h>
#include <addrspace.h>
/*
#define LEVEL_CAPACITY 1024 
#define LEVEL_ONE_MASK 11111111110000000000000000000000
#define LEVEL_TWO_MASK 00000000001111111111000000000000
 */
/*
struct pte
{
	paddr_t paddress;
}

struct ltable
{
	struct pte* pteaddress[LEVEL_CAPACITY];
};

struct ptable
{
	struct ltable* firstlevel[LEVEL_CAPACITY];
};
*/
struct pte
{
	vaddr_t vaddr;
	paddr_t paddr;
	bool swapped;
	unsigned diskaddr;
};

struct node
{
	struct pte* ptentry;
	struct node* next;
};

struct lock* copylock;
/*
struct node* head;
struct node* tail;
*/
/*
struct ptable* initializepagetable();
struct pte* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct ptable* pagetable);
void deletepagetableentry(vaddr_t vaddr);
struct pte* getpagetableentry(vaddr_t vaddr);
*/

int LRU(unsigned long npages);
paddr_t swapout(unsigned long npages);
int swapin(struct pte* ptentry,struct addrspace* addr);
void stabilizenewallocation(struct pte* ptentry,struct addrspace* as);


struct node* initializepagetable(void);
struct node* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct node* tail,struct addrspace* addr);
void deletepagetableentry(vaddr_t vaddr,struct node* head,struct addrspace* addr);
struct node* getpagetableentry(vaddr_t vaddr,struct node* head,struct addrspace* addr);
struct node* getpagetableentrywithpaddr(paddr_t paddr,struct node* head,struct addrspace* addr);
struct node* procaddpagetableentries(vaddr_t vaddr,unsigned index,struct node* tail,struct addrspace* as);

