#include <stdio.h>

#include "runtime/general/pid.h"

int 
runtime_command_pid(struct command_args* cmd_args)
{
	printf("%d\n", *(cmd_args->pid));
	return 0;
}

