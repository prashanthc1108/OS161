#include<types.h>
#include<proc.h>
#include<limits.h>
#include <synch.h>

struct proctable
{
	struct lock* proclock;
        struct proc* pt[PID_MAX];
};
/*
struct kernbuffer
{
//	struct lock* bufflock;
	char* args;
	int count;
//	size_t* len[4*PATH_MAX];
};
void allocatebuff(void);
void concatenation(char* w1);
void resetbuffer(void);
void initializekernbuffer(void);
struct kernbuffer* kernbuff;*/
struct proctable*  processtable;
struct proc* getprocessforPID(pid_t pid);
void initializeproctable(void); 
pid_t getPID(struct proc* p);
pid_t getprocessPID(void);
void deleteprocess(pid_t pid);
