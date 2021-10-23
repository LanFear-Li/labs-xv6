#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BYTE_SIZE 1

int main(int argc, char *argv[]) {
	int parent[2], child[2];
	pipe(parent);
	pipe(child);

	char BYTE[1] = {'*'};	

	if (fork() == 0) {
		/* in child process */
		int pid = getpid();
		printf("%d: received ping\n", pid);
		write(child[1], BYTE, BYTE_SIZE);	
		
	} else {
		/* in parent process */
		write(parent[1], BYTE, BYTE_SIZE);
		
		int pid = getpid();
		char buffer[BYTE_SIZE];
		read(child[0], buffer, BYTE_SIZE);
		
		printf("%d: received pong\n", pid);	
	}
	exit(0);
}
