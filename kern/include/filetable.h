#include <vnode.h>
#include <synch.h>
#define MAX_FT 64



struct mfilehandle
{
	struct lock* handlelock;
        char path[32];
        int flag;
        off_t offset;
        struct vnode* v_node;
        int refcount;
};

struct filetable
{
        struct mfilehandle* ft[MAX_FT];
};

int isfdvalidforDUP2(int fd, int isOrigin);
int clonefd(int ofd, int dfd);
void setstdfilehandle(void);
struct mfilehandle* getfilehandle(char *path, int openflags,int* ret);
struct filetable* createFileTable(void);
int getfd(char *path, int openflags,int* ret);
int closefd(int fd);
struct mfilehandle* getfhforfd(int fd);
int isfdvalid(int fd,int openflags);
//copy ft1 to ft2
void copyft(struct filetable* ft1,struct filetable* ft2);
