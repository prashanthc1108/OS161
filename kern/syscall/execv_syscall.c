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

char* kernbuffer;
//int kernelcount;
int len;
int
sys___execv(userptr_t progname, userptr_t args)
{
//	kernelcount = 0;
 	int result=0;
        char progpath[32];
	size_t actual_len;
        result = copyinstr(progname,progpath,PATH_MAX,&actual_len);
	if(result==EFAULT)
                return EFAULT;
	char prog[actual_len];
	strcpy(prog,progpath);
	
//	allocatebuff();
	if(kernbuffer==NULL)
		kernbuffer=(char*)kmalloc(ARG_MAX);

//	strcpy(kernbuff->args,"911");	
//	char* curthread->t_proc->kernBuff[ARG_MAX];
//	size_t* curthread->t_proc->argLen[ARG_MAX];
	int count = 0;
//	char* delimiter = kmalloc(sizeof(char));
	userptr_t* address;
	size_t* temps;
//        strcpy(delimiter,"~");
	while(1)
	{
//		concatenation(delimiter);
		strcat(kernbuffer,"~");
//		kernbuffer[kernelcount]='~';
//		kernelcount++;
		address = (userptr_t*)kmalloc(sizeof(userptr_t));
		if(address==NULL)
			return ENOMEM;
		result = copyin(args,address,sizeof(userptr_t));	
		if(result==EFAULT)
			return EFAULT;
		if(*address==NULL)
			break;
//		char* temp = (char*)kmalloc(sizeof(char)*ARG_MAX);
//		if(temp==NULL)
//			return ENOMEM;
		temps = (size_t*)kmalloc(sizeof(size_t));
		if(temps==NULL)
			return ENOMEM;
		result = copyinstr(*address,kernbuffer+strlen(kernbuffer),ARG_MAX,temps);
		if(result==EFAULT)
			return EFAULT;
//		*kernbuff->len[count] = *temps;
//		if(kernbuff->args[count]!=NULL)
//			kfree(kernbuff->args[count]);
//		kernbuff->args[count] = (char*)kmalloc(sizeof(char)*(*temps)); 

//		char* w1 = kmalloc(*temps);	

//		strcpy(w1,temp);		
//		concatenation(delimiter);
//		concatenation(temp);
//		kfree(temp);
//		kernelcount+=*temps-1;
		kfree(temps);
		kfree(address);
		//kprintf("%s\n",curthread->t_proc->kernBuff[count]);
		//kprintf("%d\n",*curthread->t_proc->argLen[count]);
		count++;
//		kprintf("%d\n",count);
		args=args+sizeof(struct userptr*);	
	}

	len = count;
	kprintf("%d\n",len);
	kprintf("%d\n",strlen(kernbuffer));
//	kfree(delimiter);
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

/* Open the file. */
	result = vfs_open(prog, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

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
	as_activate();
	as_destroy(oldas);


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
	vaddr_t argptr[count];
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
			copyout(nulchar,(userptr_t)stackptr,sizeof(char));
		}
		for(int k=length-1;k>=0;k--){
			//stackptr --;
			stackptr -= sizeof(char);
			//memcpy(stackptr,curthread->t_proc->kernBuff[i]+k,sizeof(char));
			copyout(arg+k,(userptr_t)stackptr,sizeof(char));
		}
		argptr[i]=stackptr;
		i++;
		arg = strtok_r(NULL,"~",&context);
//		kprintf("%d\n",i);

	}
//	(void)argptr;
	stackptr -= sizeof(vaddr_t);
	//stackptr = kmalloc(sizeof(vaddr_t));
	//memcpy(stackptr,"\0\0\0\0",sizeof(vaddr_t));
	copyout("\0\0\0\0",(userptr_t)stackptr,sizeof(vaddr_t));
	
	for(int i = count-1; i>=0; i--){
		stackptr -= sizeof(vaddr_t);
		//stackptr = kmalloc(sizeof(vaddr_t));
		//memcpy(stackptr,argptr[i],sizeof(vaddr_t));
		copyout(&argptr[i],(userptr_t)stackptr,sizeof(vaddr_t));
	}


	
	memset(kernbuffer,'\0',ARG_MAX);
	/* Warp to user mode. */	
	enter_new_process(count /*argc*/, (userptr_t)stackptr /*userspace addr of argv*/,
			NULL /*userspace addr of environment*/,
			stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
	
	
}

