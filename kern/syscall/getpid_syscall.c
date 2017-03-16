#include <types.h>
#include <lib.h>
#include <copyinout.h>
#include <syscall.h>
#include <kern/errno.h>
#include <proctable.h>

int sys___getPID(int* retval)
{
	*retval = getprocessPID();
	return 0;
}

