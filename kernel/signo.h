// Signal number

#define SIG_IGN     ((sigfunc_t) (-1))
#define SIG_DFL     ((sigfunc_t) (-2))

#define SIGHUP      1       // default: kill the process
#define SIGINT      2       // default: kill the process
#define SIGQUIT     3       // default: kill the process
#define SIGILL      4       // default: kill the process
#define SIGTRAP     5       // default: kill the process
#define SIGABRT     6       // default: kill the process
#define SIGBUS      7       // default: kill the process
#define SIGFPE      8       // default: kill the process
#define SIGKILL     9       // always: kill the process
#define SIGUSR1     10      // default: kill the process
#define SIGSEGV     11      // default: kill the process
#define SIGUSR2     12      // default: kill the process
#define SIGPIPE     13      // default: kill the process
#define SIGALRM     14      // default: kill the process
#define SIGTERM     15      // default: kill the process
#define SIGSTKFLT   16      // default: kill the process
#define SIGCHLD     17      // default: ignore
#define SIGCONT     18      // default: ignore
#define SIGSTOP     19      // always: kill the process
