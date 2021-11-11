## XV6 Labs - for Operating System

Source: https://pdos.csail.mit.edu/6.828/2021/xv6.html

### Lab 4 traps

#### Backtrace

Track function calls on the current user stack. One should check the stack frame structure and exploit the frame pointer.

#### Alarm

Set a timer for CPU tickers and trigger a user-defined function regularly. This task travels thoroughly from user to kernel and back to user. 

Notice that in kernel(traps.c/usertrap) set the  `p->trapframe->epc`to the trigger function pointer `p->handler`,the user trapframe should be restored manually. This represents for adding a new space in `struct proc`for the storage.

