// TTY Device

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "signo.h"

#define TTY_N   2

// map tty id to console id
static int tty2cons[] = {
        [0] = 0,
        [1] = 1
};

uint64 sys_sctty(void) {
    struct proc *p = myproc();
    int new_tty;

    if (argint(0, &new_tty) < 0 ||
        new_tty < -1 || new_tty >= TTY_N) {
        return -1;
    }

    set_tty(p, new_tty);

    return 0;
}

int ttyread(int minor, int user_dst, uint64 dst, int n) {
    struct proc *p = myproc();
    int tty;

    (void) minor;

    if (!p || !devsw[CONSOLE].read) {
        return -1;
    }

    tty = get_tty(p);
    if (tty < 0 || tty >= TTY_N) {
        return -1;
    }

    return devsw[CONSOLE].read(tty2cons[tty], user_dst, dst, n);
}

int ttywrite(int minor, int user_src, uint64 src, int n) {
    struct proc *p = myproc();
    int tty;

    (void) minor;

    if (!p || !devsw[CONSOLE].write) {
        return -1;
    }

    tty = get_tty(p);
    if (tty < 0 || tty >= TTY_N) {
        return -1;
    }

    return devsw[CONSOLE].write(tty2cons[tty], user_src, src, n);
}

void ttyintr(int cons, int ch) {
    int i, signo;

    switch (ch) {
        case 'C':
            signo = SIGINT;
            break;

        case 'Z':
            signo = SIGTERM;
            break;

        default:
            return;
    }

    for (i = 0; i < TTY_N; i++) {
        if (tty2cons[i] != cons) {
            continue;
        }

        signal_tty(i, signo);
    }
}

void ttyinit() {
    devsw[TTY].read = ttyread;
    devsw[TTY].write = ttywrite;
}
