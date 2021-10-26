// IPC - Signal Mechanism

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "signal.h"
#include "signo.h"

static sigfunc_t default_func[SIGNAL_N] = {
    [0 ... (SIGNAL_N - 1)] = SIGFN_KILL,
    [SIGCHLD] = SIGFN_IGN,
    [SIGCONT] = SIGFN_IGN
};

static int setup_frame() {
    struct proc *p = myproc();
    struct sigframe sgf;
    uint64 frame;
    int ret;

    sgf.epc = p->trapframe->epc;
    sgf.ra = p->trapframe->ra;
    sgf.gp = p->trapframe->gp;
    sgf.tp = p->trapframe->tp;
    sgf.t0 = p->trapframe->t0;
    sgf.t1 = p->trapframe->t1;
    sgf.t2 = p->trapframe->t2;
    sgf.s0 = p->trapframe->s0;
    sgf.s1 = p->trapframe->s1;
    sgf.a0 = p->trapframe->a0;
    sgf.a1 = p->trapframe->a1;
    sgf.a2 = p->trapframe->a2;
    sgf.a3 = p->trapframe->a3;
    sgf.a4 = p->trapframe->a4;
    sgf.a5 = p->trapframe->a5;
    sgf.a6 = p->trapframe->a6;
    sgf.a7 = p->trapframe->a7;
    sgf.s2 = p->trapframe->s2;
    sgf.s3 = p->trapframe->s3;
    sgf.s4 = p->trapframe->s4;
    sgf.s5 = p->trapframe->s5;
    sgf.s6 = p->trapframe->s6;
    sgf.s7 = p->trapframe->s7;
    sgf.s8 = p->trapframe->s8;
    sgf.s9 = p->trapframe->s9;
    sgf.s10 = p->trapframe->s10;
    sgf.s11 = p->trapframe->s11;
    sgf.t3 = p->trapframe->t3;
    sgf.t4 = p->trapframe->t4;
    sgf.t5 = p->trapframe->t5;
    sgf.t6 = p->trapframe->t6;

    // Save signal frame onto the user stack
    frame = p->trapframe->sp -= sizeof(sgf);
    ret = either_copyout(1, frame, &sgf, sizeof(sgf));
    if (ret < 0) {
        goto out;
    }

out:
    return ret;
}

int signal_init(struct proc *p) {
    struct sigdesc *desc;
    struct sigaction *actions;
    struct sigqueue *fifo;
    int i;

    desc = p->sigdesc = kalloc();
    if (!desc) {
        return -1;
    }
    actions = desc->sighand.actions;
    fifo = &desc->sigqueue;

    fifo->n = 0;
    fifo->head = fifo->tail = 0;

    for (i = 0; i < SIGNAL_N; i++) {
        actions[i].fn = default_func[i];
        actions[i].restorer = 0;
    }

    initlock(&desc->lock, "signal desc lock");

    return 0;
}

int signal_copy(struct proc *newp, struct proc *oldp) {
    if (!oldp->sigdesc || !newp->sigdesc) {
        return -1;
    }
    newp->sigdesc->sighand = oldp->sigdesc->sighand;
    return 0;
}

void signal_deinit(struct proc *p) {
    struct sigdesc *desc = p->sigdesc;
    struct sigpend *pend, *next;
    struct sigqueue *fifo;
    int n = 0;

    if (!desc) {
        return;
    }

    fifo = &desc->sigqueue;
    for (pend = fifo->head; pend; pend = next) {
        if (desc->sighand.actions[pend->signo].fn != SIGFN_IGN) {
            n++;
        }
        next = pend->next;
        kfree(pend);
    }

    if (n != fifo->n) {
        panic("wrong number of signals");
    }

    kfree(desc);
    p->sigdesc = 0;
}

void signal_deliver() {
    struct proc *p = myproc();
    struct sigdesc *desc = p->sigdesc;
    struct sigaction *actions = desc->sighand.actions, sa;
    struct sigqueue *fifo = &desc->sigqueue;
    struct sigpend *next_pend;
    int ret, signo;

    // Skip SIGFN_IGN first
    do {
        // Get a pending signal
        acquire(&desc->lock);
        if (!fifo->head) {
            // No pending signals
            release(&desc->lock);
            return;
        }
        signo = fifo->head->signo;
        next_pend = fifo->head->next;
        kfree(fifo->head);
        fifo->head = next_pend;
        if (!fifo->head) {
            fifo->tail = 0;
        }
        // Get func and restorer
        sa = actions[signo];
        if (sa.fn != SIGFN_IGN) {
            fifo->n--;
        }
        release(&desc->lock);
    } while (sa.fn == SIGFN_IGN);

    // Some special handlers
    if (sa.fn == SIGFN_KILL) {
        acquire(&p->lock);
        p->xstate.term_sig = signo;
        release(&p->lock);
        exit(255);
    }
    
    // Setup the signal frame
    ret = setup_frame();
    if (ret < 0) {
        return;
    }

    // Setup signo, restorer and pc
    p->trapframe->a0 = signo;
    p->trapframe->ra = (uint64) sa.restorer;
    p->trapframe->epc = (uint64) sa.fn;
}

// The caller must hold the process lock
int signal_send(struct proc *p, int signo) {
    struct sigdesc *desc = p->sigdesc;
    struct sigqueue *fifo;
    struct sigaction *sa;
    struct sigpend *pend;

    if (!desc) {
        panic("signal desc not found");
    }

    // FIXME: We need a small object allocator
    pend = kalloc();
    if (!pend) {
        return -1;
    }
    pend->signo = signo;
    pend->next = 0;

    acquire(&desc->lock);

    sa = &desc->sighand.actions[signo];
    if (sa->fn == SIGFN_KILL) {
        kfree(pend);
        p->xstate.term_sig = signo;
        p->killed = 1;
        goto wakeup;
    } else if (sa->fn == SIGFN_IGN) {
        kfree(pend);
        goto out;
    }

    fifo = &desc->sigqueue;
    if (!fifo->head) {
        fifo->head = pend;
    } else {
        fifo->tail->next = pend;
    }
    fifo->tail = pend;
    fifo->n++;

wakeup:
    if (p->state == SLEEPING) {
        // Wake process from sleep().
        p->state = RUNNABLE;
    }

out:
    release(&desc->lock);
    return 0;
}

uint64 sys_sigreturn(void) {
    // Must be called by user-space restorer
    struct proc *p = myproc();
    struct sigframe sgf;
    uint64 frame;
    int ret;

    // Copy signal frame from the user stack
    frame = p->trapframe->sp;
    ret = either_copyin(&sgf, 1, frame, sizeof(sgf));
    if (ret < 0) {
        goto out;
    }
    
    // Reset trapframe
    p->trapframe->epc = sgf.epc;
    p->trapframe->ra = sgf.ra;
    p->trapframe->gp = sgf.gp;
    p->trapframe->tp = sgf.tp;
    p->trapframe->t0 = sgf.t0;
    p->trapframe->t1 = sgf.t1;
    p->trapframe->t2 = sgf.t2;
    p->trapframe->s0 = sgf.s0;
    p->trapframe->s1 = sgf.s1;
    p->trapframe->a0 = sgf.a0;
    p->trapframe->a1 = sgf.a1;
    p->trapframe->a2 = sgf.a2;
    p->trapframe->a3 = sgf.a3;
    p->trapframe->a4 = sgf.a4;
    p->trapframe->a5 = sgf.a5;
    p->trapframe->a6 = sgf.a6;
    p->trapframe->a7 = sgf.a7;
    p->trapframe->s2 = sgf.s2;
    p->trapframe->s3 = sgf.s3;
    p->trapframe->s4 = sgf.s4;
    p->trapframe->s5 = sgf.s5;
    p->trapframe->s6 = sgf.s6;
    p->trapframe->s7 = sgf.s7;
    p->trapframe->s8 = sgf.s8;
    p->trapframe->s9 = sgf.s9;
    p->trapframe->s10 = sgf.s10;
    p->trapframe->s11 = sgf.s11;
    p->trapframe->t3 = sgf.t3;
    p->trapframe->t4 = sgf.t4;
    p->trapframe->t5 = sgf.t5;
    p->trapframe->t6 = sgf.t6;
    p->trapframe->sp += sizeof(sgf);

out:
    return ret;
}

uint64 sys_sigaction(void) {
    struct proc *p = myproc();
    struct sigdesc *desc = p->sigdesc;
    struct sigaction *actions = desc->sighand.actions, *sa;
    sigfunc_t fn, restorer;
    struct sigqueue *fifo;
    struct sigpend *pend;
    int signo, delta = 0;

    if (argint(0, &signo) < 0 ||
        argaddr(1, (uint64 *) &fn) < 0 ||
        argaddr(2, (uint64 *) &restorer) < 0) {
        return -1;
    }

    // validity check
    if (signo < 1 || signo >= SIGNAL_N ||
        signo == SIGKILL || signo == SIGSTOP) {
        return -1;
    }

    // convert fn to kernel sigfunc
    if (fn == SIG_IGN) {
        fn = SIGFN_IGN;
    } else if (fn == SIG_DFL) {
        fn = default_func[signo];
    }

    acquire(&desc->lock);

    sa = &actions[signo];

    if (fn == SIGFN_IGN && sa->fn != SIGFN_IGN) {
        /* from !ign to ign */
        delta = -1;
    } else if (fn != SIGFN_IGN && sa->fn == SIGFN_IGN) {
        /* from ign to !ign */
        delta = 1;
    }

    // set func and restorer
    sa->fn = (sigfunc_t) fn;
    sa->restorer = (sigrestorer_t) restorer;

    if (delta) {
        // update the counter for pending signals
        fifo = &desc->sigqueue;
        for (pend = fifo->head; pend; pend = pend->next) {
            if (pend->signo == signo) {
                fifo->n += delta;
            }
        }
    }

    release(&desc->lock);

    return 0;
}

int signal_pending() {
    struct proc *p = myproc();
    struct sigdesc *desc = p->sigdesc;
    int pending;
    acquire(&desc->lock);
    pending = desc->sigqueue.n;
    release(&desc->lock);
    return pending;
}
