#include "kernel/syscall.h"

.globl sigrestorer
sigrestorer:
    li a7, SYS_sigreturn
    ecall
