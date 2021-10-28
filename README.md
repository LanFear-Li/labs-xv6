## XV6 Labs - for Operating System

Source: https://pdos.csail.mit.edu/6.828/2021/xv6.html

### Lab 2 System Calls

#### **sys_trace**

System call **trace** Tracing target system calls at the aimed process and its child process. The MASK validate bits added at process structure are 32-binary bits, the k-th bit stands for the k-th system call whether is being traced.

#### **sys_sysinfo**

System call **sysinfo** get the free-memory-bytes and in-use processes amount of xv6. Adding function **freemem_size** and **proc_num** at kernel/kalloc.c and kernel/proc.c.

