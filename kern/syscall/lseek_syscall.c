#include <types.h>
#include <lib.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <uio.h>
#include <filetable.h>
#include <current.h>
#include <proc.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/seek.h>
#include <sfs.h>
#include <stat.h>

int
sys___lseek(int fd, off_t offset,int whence,int32_t* retval,int32_t* retval1)
{
	off_t temp;
        int result=0;
	kprintf("%ld",(long int)offset);       
	if(whence!=SEEK_SET&&whence!=SEEK_CUR&&whence!=SEEK_END)
		return EINVAL; 
	result = isfdvalidforDUP2(fd,1);
	if(result)
                return result;
        struct mfilehandle* fh = getfhforfd(fd);
	if(!VOP_ISSEEKABLE(fh->v_node))
	{
		return ESPIPE;
	}
	result =  isfdvalid(fd,O_WRONLY);
        if(result)
                return result;



	struct stat metadata;


	switch (whence)
	{
	case SEEK_SET:
	if(offset<0)
		return EINVAL;
	fh->offset = offset;
	break;
	case SEEK_CUR:
	temp = fh->offset+offset;
	if(temp<0)
		return EINVAL;
	fh->offset = temp;
	break;
	case SEEK_END:
	//TO DO has to be changes
	VOP_STAT(fh->v_node,&metadata);
	temp = metadata.st_size+offset;
        if(temp<0)
                return EINVAL;
        fh->offset = temp;
	break;
	}
        *retval = (int32_t)(fh->offset>>32);
	*retval1 = (int32_t)fh->offset;
        return result;
}

