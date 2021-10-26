// IPC - Signal Mechanism

#define SIGNAL_N     (sizeof(sigset_t) * 8)

#define SIGFN_IGN    ((sigfunc_t) (-1))
#define SIGFN_KILL   ((sigfunc_t) (-2))

// We allow at most 64 signals
typedef uint64 sigset_t;

// User-space signal handler/restorer function
typedef void (*sigfunc_t)(int);
typedef void (*sigrestorer_t)();

// Signal action
struct sigaction {
    sigfunc_t fn;
    sigrestorer_t restorer;
};

// Signal pending queue entry
struct sigpend {
    int signo;
    struct sigpend *next;
};

// Per-process signal pending queue
struct sigqueue {
    // Number of pending signals whose handlers are not SIG_IGN
    int n;
    struct sigpend *head, *tail;
};

// Per-process signal handler
struct sighand {
    struct sigaction actions[SIGNAL_N];
};

// Per-process signal descriptor
struct sigdesc {
    struct spinlock lock;
    struct sigqueue sigqueue;
    struct sighand sighand;
};

// Signal frame
struct sigframe {
    uint64 epc;
    uint64 ra;
    uint64 gp;
    uint64 tp;
    uint64 t0;
    uint64 t1;
    uint64 t2;
    uint64 s0;
    uint64 s1;
    uint64 a0;
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 a4;
    uint64 a5;
    uint64 a6;
    uint64 a7;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
    uint64 t3;
    uint64 t4;
    uint64 t5;
    uint64 t6;
};

int signal_init(struct proc *p);
int signal_copy(struct proc *, struct proc *);
void signal_deinit(struct proc *p);
int signal_send(struct proc *p, int signo);
void signal_deliver();
int signal_pending();
