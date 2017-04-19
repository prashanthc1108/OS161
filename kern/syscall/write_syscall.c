#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <uio.h>
#include <filetable.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>
#include <sfs.h>


int
sys___write(int fd, userptr_t buffer,size_t len,int32_t* retval)
{
	int result=0;
	char* localbuffer = kmalloc(sizeof(char)*len);
	size_t actual_len;
	if(fd<0||fd>MAX_FT)
		{
		kfree(localbuffer);
		return EBADF;
		}	
//	if(buffer==NULL||(buffer<=(userptr_t)0x40000000||buffer>=(userptr_t)0x80000000))
//                return EFAULT;
        result = newcopycheck(buffer,len,&actual_len);
        if(result)
        {
 //       kprintf("valid");
		kfree(localbuffer);
                return result;
       }
	(void)actual_len;
	result = copyin(buffer,localbuffer,len);
	if(result==EFAULT)
	{
		kfree(localbuffer);
                return EFAULT;
	}
	kfree(localbuffer);        
//TO DO has to be changed done only to pass console test
/*	if(fd==1)
	{
		struct mfilehandle *fh = getfhforfd(fd);
		lock_acquire(fh->handlelock);
		size_t i;
		for (i=0; i<len; i++) {
        	        putch(localbuffer[i]);
	       	 }
		lock_release(fh->handlelock);
		*retval = i;
		return result;
	}

	if(fd==2)
        {
                struct mfilehandle *fh = getfhforfd(fd);
                lock_acquire(fh->handlelock);
                size_t i;
		for (i=0; i<len; i++) {
                        putch(localbuffer[i]);
                 }
                lock_release(fh->handlelock);
		*retval = i;
                return result;
        }
*/
        result =  isfdvalid(fd,O_WRONLY);

        if(result)
                return result;


        struct mfilehandle* fh = getfhforfd(fd);
        struct iovec iov;
        struct uio u;
//	struct sfs_vnode *sv = fh->v_node->vn_data;
//	kprintf("%ld",(long int)sv->sv_i.sfi_size);
	lock_acquire(fh->handlelock);
        iov.iov_ubase = buffer;
        iov.iov_len = len;           // length of the memory space
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_resid = len;          // amount to read from the file
        u.uio_offset = fh->offset;
        u.uio_segflg = UIO_USERSPACE;
        u.uio_rw = UIO_WRITE;
        u.uio_space = curthread->t_proc->p_addrspace;

        result = VOP_WRITE(fh->v_node, &u);
	*retval = u.uio_offset-fh->offset;
	fh->offset = u.uio_offset;
	lock_release(fh->handlelock);
        return result;
}
