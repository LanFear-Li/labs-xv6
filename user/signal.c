#include "kernel/types.h"
#include "kernel/signo.h"
#include "user/user.h"

static void handler(int signo) {
    wait(0);
    printf("handle child exit: wait\n");
}

int main() {
    char ch;
    sigaction(SIGCHLD, handler, sigrestorer);

    if (fork() == 0) {
        read(0, &ch, 1);
        exit(0);
    }

    while(1);
    exit(0);
}
