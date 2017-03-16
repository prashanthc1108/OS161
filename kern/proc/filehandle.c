#include <types.h>
#include <lib.h>
#include <vnode.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <current.h>
#include <filetable.h>
#include <array.h>
#include <proc.h>
#include <thread.h>
#include <cpu.h>
#include <kern/errno.h>

void setstdfilehandle()
{
	struct mfilehandle* fh1;
        fh1 = kmalloc(sizeof(struct mfilehandle));
	struct mfilehandle* fh2;
        fh2 = kmalloc(sizeof(struct mfilehandle));
	struct mfilehandle* fh3;
        fh3 = kmalloc(sizeof(struct mfilehandle));
        struct vnode* v_node1;
	
	char buf1[32];
	strcpy(buf1,"con:");
	vfs_open(buf1,O_RDONLY,0664,&v_node1);
	struct lock*  handle_lock1 = lock_create("lock");
        fh1->handlelock = handle_lock1;
        strcpy(fh1->path,"con:");
	fh1->refcount=1;
        fh1->flag =O_RDONLY;
	fh1->offset=0;
        fh1->v_node=v_node1;
	
	struct vnode* v_node2;
	char buf2[32];
        strcpy(buf2,"con:");
	struct lock*  handle_lock2 = lock_create("lock");
        fh2->handlelock = handle_lock2;
        vfs_open(buf2,O_WRONLY,0664,&v_node2);
        strcpy(fh2->path,"con:");
        fh2->refcount=1;
        fh2->flag =O_WRONLY;
        fh2->offset=0;
        fh2->v_node=v_node2;


	struct vnode* v_node3;
	char buf3[32];
        strcpy(buf3,"con:");
        struct lock*  handle_lock3 = lock_create("lock");
        fh3->handlelock = handle_lock3;
	vfs_open(buf3,O_WRONLY,0664,&v_node3);
        strcpy(fh3->path,"con:");
        fh3->refcount=1;
        fh3->flag =O_WRONLY;
        fh3->offset=0;
        fh3->v_node=v_node3;

	


	curthread->t_proc->ftab->ft[0] = fh1;
	curthread->t_proc->ftab->ft[1] = fh2;
	curthread->t_proc->ftab->ft[2] = fh3;

}

/*
void destroyfh(struct mfilehandle* fh)
{
	
}
*/
struct mfilehandle* getfilehandle(char *path, int openflags,int* ret)
{
	struct mfilehandle* fh;
	fh = kmalloc(sizeof(struct mfilehandle));
	struct vnode* v_node;
//	TO DO should handle failure

	*ret = vfs_open(path,openflags,0664,&v_node);
	if(*ret)
		return NULL;
	strcpy(fh->path,path);
	struct lock*  handle_lock = lock_create("lock");
	fh->handlelock = handle_lock;
	fh->refcount=1;
	fh->flag =openflags;
	fh->offset=0;
	fh->v_node=v_node;
	return fh; 
}
