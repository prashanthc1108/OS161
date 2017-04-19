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
#include <proctable.h>

struct filetable* createFileTable()
{	
	struct filetable* ftab = kmalloc(sizeof(struct filetable));
	if(ftab==NULL)
		return NULL;
	for (int i=0;i<MAX_FT;i++)
		ftab->ft[i] = NULL;
	return ftab;
}

int isfdvalid(int fd,int openflags)
{
	struct mfilehandle* fh;
        for (int i=0;i<MAX_FT;i++)
        {       
		if(fd!=i)
                        continue;
                fh =curthread->t_proc->ftab->ft[i];
                if(fh!=NULL)
                {
			if(openflags == O_RDONLY && (fh->flag&1)==1 )
			{
				return EBADF;
			}
			if(openflags == O_WRONLY && ((fh->flag&3)!=2&&(fh->flag&3)!=1))
			{
				kprintf("here");
				return EBADF;
			}
			return 0;       
                }
		else
		{
			return EBADF;
		}
        }
	return EBADF;
}

struct mfilehandle* getfhforfd(int fd)
{
	return curthread->t_proc->ftab->ft[fd];	
}

int getfd(char *path, int openflags,int* ret)
{
	int len = MAX_FT;
	struct mfilehandle* fh;
	struct mfilehandle* new_fh =  getfilehandle(path,openflags,ret);
	if(new_fh == NULL)
	{
		return -1;
	}
	int i=3;
	for (i=3;i<len;i++)
	{
		fh = curthread->t_proc->ftab->ft[i];
                if(fh == NULL)
                        break;
	}
	curthread->t_proc->ftab->ft[i]=new_fh;
	return i;
	
}

int closefd(int fd)
{
	struct mfilehandle* fh;
	fh =curthread->t_proc->ftab->ft[fd];
	if(fh==NULL)
	{
		return EBADF;
	
	}
	else
	{
		deletefh(fh);
		curthread->t_proc->ftab->ft[fd] = NULL;
		return  0;
	}
}

void deleteFT(struct filetable* ftab)
{

	for (int i=3;i<MAX_FT;i++)
	{
		if(ftab->ft[i]!=NULL){
			deletefh(ftab->ft[i]);
		}
	}

	kfree(ftab);

}



void copyft(struct filetable* ft1,struct filetable* ft2)
{
	for (int i=0;i<MAX_FT;i++)
	{
		ft2->ft[i]=ft1->ft[i];	
		if(ft1->ft[i]!=NULL)
		{  
		lock_acquire(ft1->ft[i]->handlelock);
			ft1->ft[i]->refcount++;
		lock_release(ft1->ft[i]->handlelock);
		}
	}
}


int isfdvalidforDUP2(int fd, int isOrigin)
{
struct mfilehandle* fh;
	
	if(fd>=0 && fd<MAX_FT)
	{       
		fh =curthread->t_proc->ftab->ft[fd];
		if(isOrigin){
			if(fh!=NULL)
			{
				return 0;       
			}
			else
			{
				return EBADF;
			}
		}else{
			return 0;       

		}
	}

	return EBADF;
}



int clonefd(int ofd, int dfd){
	if(ofd==dfd)
	    return 0;	
	if(curthread->t_proc->ftab->ft[dfd]!=NULL){
		closefd(dfd);
	}
	curthread->t_proc->ftab->ft[dfd] = curthread->t_proc->ftab->ft[ofd];
lock_acquire(curthread->t_proc->ftab->ft[dfd]->handlelock);	
	curthread->t_proc->ftab->ft[dfd]->refcount++;
lock_release(curthread->t_proc->ftab->ft[dfd]->handlelock);
	return 0;
}
