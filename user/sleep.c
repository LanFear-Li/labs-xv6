#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (!argv[1]) {
        printf("error: no arguments\n");
	    exit(1);
    }

    int secs = atoi(argv[1]);
    sleep(secs);
    exit(0);
}
