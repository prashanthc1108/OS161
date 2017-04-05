#include<types.h>
#include <lib.h>
#include<proctable.h>
#include<current.h>
#include <synch.h>
/*

void concatenation(char* w1)
{
//      char* w3 = kmalloc(strlen(w1)+strlen(w2)+1);
//      strcpy(w3,w1);
//        strcat(kernbuff->args,w1);
}

void resetbuffer()
{


//memset(kernbuff->args,'\0',ARG_MAX);
//	for (int i=0;i<4*PATH_MAX;i++)
  //              {
               // kfree(kernbuff->args);
//		kernbuff->args=kmalloc(ARG_MAX);
//		kfree(kernbuff->len[i]);
//                kernbuff->len[i] =NULL;
//		}

}

void allocatebuff()
{
//	if(kernbuff->args==NULL)
//		kernbuff->args = kmalloc(ARG_MAX);
}

void initializekernbuffer()
{
  //      kernbuff =  kmalloc(sizeof(struct kernbuffer));
//	kernbuff->args = NULL;
//	kernbuff->args=kmalloc(ARG_MAX);
//        for (int i=0;i<4*PATH_MAX;i++)
//		{
//                kernbuff->args[i] = (char*)kmalloc(sizeof(char)*ARG_MAX);
//		kernbuff->len[i] = NULL;
//		}
    //    kernbuff->bufflock=lock_create("buff_lock");
}
*/
void initializeproctable()
{
	struct proctable* pt;
	pt =  kmalloc(sizeof(struct proctable));
	processtable = pt;
	kprintf("\n%d\n",PID_MAX);
        for (pid_t i=0;i<PID_MAX;i++)
                processtable->pt[i] = NULL;
	processtable->proclock=lock_create("proc_lock");
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
//pid_t i;
//        for (i=2;i<__PID_MAX;i++)
//        {
//                if(processtable->pt[i]->PID==pid)
//                break;
//        }
//        if(i==__PID_MAX)
//                return NULL;
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

