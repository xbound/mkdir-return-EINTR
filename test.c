#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/futex.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
//can work on arm64
int main(){
	int pid,cont=0;
	long r,r1,r2;
	struct user_regs_struct urs;
	struct iovec iv;
	uint32_t *start;
	start=mmap(NULL,4,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
	*start=0;
	pid=fork();
	if(pid){
		r=ptrace(PTRACE_SEIZE,pid,NULL,NULL);
		r=ptrace(PTRACE_INTERRUPT,pid,NULL,NULL);
		*start=1;
		syscall(SYS_futex,&start,FUTEX_WAKE,1,NULL,NULL,0);
		wait(NULL);
		while(1){
		r=ptrace(PTRACE_SYSCALL,pid,NULL,NULL);
		if(wait(NULL)<0)return 0;
		iv.iov_base=&urs;
		iv.iov_len=sizeof urs;
		r=ptrace(PTRACE_GETREGSET,pid,1,&iv);
		r1=urs.regs[8];
		if(r1==SYS_mkdirat){
			r2=urs.regs[1];
			urs.regs[1]=0;
			r=ptrace(PTRACE_SETREGSET,pid,1,&iv);
		}
		r=ptrace(PTRACE_SYSCALL,pid,NULL,NULL);
		if(wait(NULL)<0)return 0;
		if(r1==SYS_mkdirat){
		r=ptrace(PTRACE_GETREGSET,pid,1,&iv);
		urs.regs[1]=r2;
		urs.regs[0]=(long)-EINTR;
		r=ptrace(PTRACE_SETREGSET,pid,1,&iv);
		}
		}
	}
	else{
			syscall(SYS_futex,start,FUTEX_WAIT,0,NULL,NULL,0);
			if((mkdir("hh",0777))<0){
				perror("mkdir");
			}else fprintf(stderr,"mkdir ok\n");
	}
}
