// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *sh_argv[] = { "sh", 0 };

int run(int ctty, char **argv) {
    int ppid, pid;

    ppid = fork();

    if (ppid < 0) {
        printf("init: fork failed\n");
        exit(1);
    } else if (ppid == 0) {
        for (;;) {
            printf("init: starting sh\n");
            pid = fork();

            if (pid < 0) {
                printf("init: fork failed\n");
                exit(1);
            } else if (pid == 0) {
                sctty(ctty);
                exec(argv[0], argv);
                printf("init: exec sh failed\n");
                exit(1);
            }

            if (wait((int *) 0) < 0) {
                printf("init: wait returned an error\n");
                exit(1);
            }
        }
    }

    return ppid;
}

int
main(void)
{
  if(open("tty", O_RDWR) < 0){
    mknod("tty", TTY, 0);
    open("tty", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  if(open("console0", O_RDWR) < 0){
    mknod("console0", CONSOLE, 0);
    open("console0", O_RDWR);
  }

  if(open("console1", O_RDWR) < 0){
    mknod("console1", CONSOLE, 1);
    open("console1", O_RDWR);
  }

  run(0, sh_argv);
  run(1, sh_argv);

  for (;;) {
    // this call to wait() returns if the shell exits,
    // or if a parentless process exits.
    if (wait((int *) 0) < 0) {
      printf("init: wait returned an error\n");
      exit(1);
    }
  }
}
