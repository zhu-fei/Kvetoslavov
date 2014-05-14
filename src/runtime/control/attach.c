#include "runtime/control/attach.h"
#include "runtime/util.h"
#include "state.h"

#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <kvm.h>
#include <sys/vnode.h>
#include <libprocstat.h>

int
attach (pid_t pid)
{
	int wait_status;
	
	if (ptrace(PT_ATTACH, pid, NULL, NULL) == 0)
	{
		wait(&wait_status);
		return 0;
	}
	else 
	{
		return 1;
	}
}

int
parse_pid (char *text, pid_t *pid)
{
	long int input;
	
	errno = 0;
	input = strtol(text, NULL, 10);

	if (errno != 0)
	{
		if (errno == EINVAL)
			return 2;
		else if (errno == ERANGE)
			return 3;
		else
			return 4;
	}

	if (input < 0)
		return 1;

	*pid = input;
	return 0;
}

char*
search_exec_path_for_pid (pid_t pid)
{
	kvm_t *kvm;
	struct kinfo_proc *procs;
	int count;
	char *freepath = NULL;
	char *fullpath = NULL;
	struct procstat *ps;
	char *result;

	kvm = kvm_open(0, 0, 0, O_RDONLY, NULL);
	procs = kvm_getprocs(kvm, KERN_PROC_PID, pid, &count);

	if (count > 0)
	{
		ps = procstat_open_sysctl();
		result = (char*)malloc(255 * sizeof(char));
		memset(result, '\0', 255);
		procstat_getpathname(ps, &procs[0], result, 254);

		kvm_close(kvm);
		procstat_close(ps);
		return result;	
	}
	else
	{
		kvm_close(kvm);
		return NULL;	
	};
}

int 
runtime_command_attach (struct command_args_t *args)
{
	pid_t pid;
	int pid_parse_error;

	if ((pid_parse_error = parse_pid(args->text_args[1], &pid)) != 0)
	{
		switch (pid_parse_error)
		{
			case 1:
				fprintf(stderr, "The PID is negative.\n");
			break;

			default:
				fprintf(stderr, "Wrong PID format.\n");	
		}
		return 0;
	}

	if (runtime_process_exists(pid) == 0)
	{
		if (attach(pid) == 0)
		{
			printf("Attach was successful.\n");

			*(args->state) = RUNNING;
			*(args->pid) = pid;
			*(args->exec_path) = search_exec_path_for_pid(pid);
		}
		else
		{
			printf("Attach was unsuccessful.\n");
		}
	}
	else
	{
		printf("Process with such PID does not exist.\n");
	}

	return 0;
}

