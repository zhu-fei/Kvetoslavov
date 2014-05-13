#ifndef KVETOSLAVOV_RUNTIME_CONTROL_COMMON
#define KVETOSLAVOV_RUNTIME_CONTROL_COMMON

#include "runtime/breakpoint/breakpoint.h"

#include <unistd.h>

int 
after_wait(int wait_status, pid_t *pid, struct breakpoint_t **bp);

#endif
