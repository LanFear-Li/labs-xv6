#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N       1000
#define NULL    0

int vis[N];

void prime(int *buffer, int count, int *fd) {
    int prime = buffer[0];
    printf("prime %d\n", prime);

    for (int i = 1; i < count; i++) {
        if (buffer[i] % prime) {
            write(fd[1], buffer + i, sizeof(int));
        }
    }

    close(fd[1]);
}

int main(int argc, char *argv[]) {
    int fd[N][2], pipeline = 1;
    pipe(fd[pipeline]);

    int buffer[N];
    for (int i = 0; i < N - 1; i++) {
        buffer[i] = i + 2;
    }
    prime(buffer, N - 1, fd[pipeline]);

	int pid = fork();
    /* reduce other process */
    while (pid == 0) {
        int buffer[N];
        int count = 0;
        while (read(fd[pipeline][0], buffer + count, 4)) {
            count++;
        }

        /* close fd and continue pipeline */
        close(fd[pipeline++][0]);
        if (count == 0) {
            break;
        }

        pipe(fd[pipeline]);
        prime(buffer, count, fd[pipeline]);
        pid = fork();
    }

    while(wait(NULL) > 0);
    exit(0);
}
