#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <limits.h>
#include <copyinout.h>
//#include <proctable.h>
#include <synch.h>

static char kernbuffer[ARG_MAX];
//int kernelcount;
int len;
int
sys___execv(userptr_t progname, userptr_t args)
{
 	int result=0;
	userptr_t arg1 =args;
        char* progpath=kmalloc(PATH_MAX);
	size_t actual_len;
        result = copyinstr(progname,progpath,PATH_MAX,&actual_len);
	if(result==EFAULT)
		{
		kfree(progpath);
                return EFAULT;
		}
	char* prog=kmalloc(actual_len);
	if(prog==NULL)
                {
                kfree(progpath);
                return ENOMEM;
                }
	strcpy(prog,progpath);
	kfree(progpath);	
	int count = 0;
	userptr_t* address;
	address = (userptr_t*)kmalloc(sizeof(userptr_t));
	if(address==NULL)
		{
		kfree(prog);
		return ENOMEM;
		}
	size_t* temps;
	temps = (size_t*)kmalloc(sizeof(size_t));
	if(temps==NULL)
		{
		kfree(progpath);
		kfree(address);
		return ENOMEM;
		}
	int curpos = 0;
	while(1)
	{
	//	strcat(kernbuffer,"~");
		kernbuffer[curpos] = '~';
		curpos++;
		result = copyin(arg1,address,sizeof(userptr_t));	
		if(result==EFAULT)
			{
			 kfree(progpath);
               		 kfree(address);
			kfree(temps);	
			return EFAULT;
			}
		if(*address==NULL)
			break;
		result = copyinstr(*address,kernbuffer+curpos,ARG_MAX,temps);
		if(result==EFAULT)
			{
                         kfree(progpath);
                         kfree(address);
                        kfree(temps);
			return EFAULT;
			}
		curpos+=*temps-1;
		count++;
		arg1=arg1+sizeof(struct userptr*);	
	}

	kfree(temps);
	kfree(address);
	len = count;
	kprintf("%d\n",len);
	kprintf("%d\n",curpos);
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

/* Open the file. */
	result = vfs_open(prog, O_RDONLY, 0, &v);
	if (result) {
		kfree(prog);
		return result;
	}
	kfree(prog);
	/* We should be a new process. */
//	KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}
	/* Switch to it and activate it. */
	struct addrspace* oldas = proc_setas(as);
	as_destroy(oldas);
	as_activate();


	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	//kprintf("\n%lu\n",(long unsigned int)stackptr);	

	char* arg;
//	char* context;
	count = len;
	int i =0;
	vaddr_t* argptr = kmalloc(sizeof(vaddr_t)*count);
	char * context;
//	kprintf("%s\n",kernbuff->args);
	char* nulchar = kmalloc(sizeof(char));
	strcpy(nulchar,"\0");
	arg = strtok_r(kernbuffer,"~",&context);
	while(arg!=NULL)
	{
		int length = strlen(arg);
		int padbits = 4-length%4;
		for(int j=padbits;j>0;j--){
			//stackptr --;
			stackptr -= sizeof(char);
			//stackptr = (userptr_t *)"\0";
			//memmove(nulchar,(userptr_t)stackptr,sizeof(char));
			copyout(nulchar,(userptr_t)stackptr,sizeof(char));
		}
		for(int k=length-1;k>=0;k--){
			//stackptr --;
			stackptr -= sizeof(char);
			//memcpy(stackptr,curthread->t_proc->kernBuff[i]+k,sizeof(char));
			//memmove(arg+k,(userptr_t)stackptr,sizeof(char));
			copyout(arg+k,(userptr_t)stackptr,sizeof(char));
		}
		*(argptr+i)=stackptr;
		i++;
		arg = strtok_r(NULL,"~",&context);
//		kprintf("%d\n",i);

	}
//	(void)argptr;
	stackptr -= sizeof(vaddr_t);
	//stackptr = kmalloc(sizeof(vaddr_t));
	//memcpy(stackptr,"\0\0\0\0",sizeof(vaddr_t));
	//memmove("\0\0\0\0",(userptr_t)stackptr,sizeof(vaddr_t));
	copyout("\0\0\0\0",(userptr_t)stackptr,sizeof(vaddr_t));
	
	for(int i = count-1; i>=0; i--){
		stackptr -= sizeof(vaddr_t);
		//stackptr = kmalloc(sizeof(vaddr_t));
		//memcpy(stackptr,argptr[i],sizeof(vaddr_t));
//		memmove(&argptr[i],(userptr_t)stackptr,sizeof(vaddr_t));
		copyout(argptr+i,(userptr_t)stackptr,sizeof(vaddr_t));
	}

	kfree(argptr);
	kfree(nulchar);
	memset(kernbuffer,'\0',ARG_MAX);
	/* Warp to user mode. */	
	enter_new_process(count /*argc*/, (userptr_t)stackptr /*userspace addr of argv*/,
			NULL /*userspace addr of environment*/,
			stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
	
	
}

