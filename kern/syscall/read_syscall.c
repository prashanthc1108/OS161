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



int
sys___read(int fd, userptr_t buffer,size_t len,int32_t* retval)
{
        int result = 0;
	size_t actual_len;
//TO DO change this to the actual range. this is pathetic
	if(buffer==NULL||buffer==(userptr_t)0x40000000)
		return EFAULT;
	result = newcopycheck(buffer,len,&actual_len);
	if(result)
	{
	kprintf("valid");
                return result;
       }
	result =  isfdvalid(fd,O_RDONLY);
	if(result)
                return result;
/*	if(fd==0)
		{
		struct mfilehandle *fh = getfhforfd(fd);
                lock_acquire(fh->handlelock);	
		int ch =0 ;
		ch = getch();
		lock_release(fh->handlelock);
                result = copyout(&ch,buffer,len);
		*retval = len;
		return result;					
		
		}
*/	

	struct mfilehandle* fh = getfhforfd(fd);
	struct iovec iov;
        struct uio u;
	
        lock_acquire(fh->handlelock);

	iov.iov_ubase = buffer;
        iov.iov_len = len;           // length of the memory space
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_resid = len;          // amount to read from the file
        u.uio_offset = fh->offset;
        u.uio_segflg = UIO_USERSPACE;
        u.uio_rw = UIO_READ;
        u.uio_space = curthread->t_proc->p_addrspace;

        result = VOP_READ(fh->v_node, &u);
	
	*retval = u.uio_offset - fh->offset;
	fh->offset = u.uio_offset;	
        lock_release(fh->handlelock);

	return result;

}

