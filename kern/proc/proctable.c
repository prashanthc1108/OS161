#include<types.h>
#include <lib.h>
#include<proctable.h>
#include<current.h>
#include <synch.h>


void initializeproctable()
{
	struct proctable* pt;
	pt =  kmalloc(sizeof(struct proctable));
	processtable = pt;
        /*for (pid_t i=0;i<PID_MAX;i++)
                processtable->pt[i] = NULL;
	*/processtable->proclock=lock_create("proc_lock");
}

pid_t getPID(struct proc* p)
{
	pid_t i;
	for (i=2;i<PID_MAX;i++) 
	{
                if(processtable->pt[i]==NULL)
		break;
	}
	if(i==PID_MAX)
		return -1;
	processtable->pt[i] = p;
	return i;
}

struct proc* getprocessforPID(pid_t pid)
{
        return processtable->pt[pid];

}

void deleteprocess(pid_t pid)
{
	proc_destroy(processtable->pt[pid]);
	processtable->pt[pid] = NULL;
}

pid_t getprocessPID()
{
return curproc->PID;
}

