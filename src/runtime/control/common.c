#include <sys/types.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <machine/reg.h>
#include <fcntl.h>
#include <kvm.h>
#include <signal.h>
#include <stdio.h>

#include "runtime/control/common.h"

int 
after_wait(int wait_status, pid_t* pid, struct breakpoint** bp)
{
	struct reg regs;
	struct breakpoint* node;

	/* process-change handling routine */
	if (WIFEXITED(wait_status)) {
		fprintf(stdout, "Child exited.\n");
		return 0;
	} else if (WIFSTOPPED(wait_status)) {
		if (WSTOPSIG(wait_status) != SIGTRAP)
			printf("Program received %s.\n", sys_siglist[WSTOPSIG(wait_status)]);
			
		ptrace(PT_GETREGS, *pid, (caddr_t)&regs, 0);

		node = *bp;
		while (node != NULL) {
			if (regs.r_eip - 1 == node->addr) {
				printf("Stopped at breakpoint in file %s at line %d. ", node->path, 
				       node->line);
				break;
			}
			node = node->next;
		}

		/* at end, but not at start either
		 * means we have breakpoints, but have not stopped
		 * on one of them */
		if (node == NULL && *bp != NULL)
			printf("Stopped at unknown place. ");

		printf("(IP = 0x%X)\n", regs.r_eip);
		
		return 1;
	} else {
		return -1;
	}
}

int
runtime_process_exists(pid_t pid)
{
	kvm_t* kvm;
	struct kinfo_proc* result;
	int count;

	kvm = kvm_open(0, 0, 0, O_RDONLY, NULL);
	result = kvm_getprocs(kvm, KERN_PROC_PID, pid, &count);
	kvm_close(kvm);

	if (result != NULL)
		return 0;
	else
		return 1;
}

