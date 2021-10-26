#include "kernel/types.h"
#include "kernel/signo.h"
#include "user/user.h"

static int counter = 0;

static void handler(int signo) {
    sigaction(SIGUSR1, SIG_IGN, sigrestorer);
    printf("received: %d, signo: %d\n", counter++, signo);
    sigaction(SIGUSR1, handler, sigrestorer);
}

int main() {
    char ch;
    sigaction(SIGUSR1, handler, sigrestorer);
    printf("Press enter to exit!\n");
    while (~read(1, &ch, 1));
    exit(0);
}
