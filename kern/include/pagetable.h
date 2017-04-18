#include <vm.h>

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
};

struct node
{
	struct pte* ptentry;
	struct node* next;
};
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

struct node* initializepagetable(void);
struct node* addpagetableentries(vaddr_t vaddr,paddr_t paddr,struct node* tail);
void deletepagetableentry(vaddr_t vaddr,struct node* head);
struct node* getpagetableentry(vaddr_t vaddr,struct node* head);


