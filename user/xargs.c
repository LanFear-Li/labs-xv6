#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define BUFFER_SIZE     1024
#define NULL            ((void *) 0)
#define STDIN           0

int get_line(char *line_buffer) {
    int count = 0;
    if (read(STDIN, line_buffer + count, 1) == 0) {
        return 0;
    }

    while (line_buffer[count] != '\n') {
        count += 1;
        read(STDIN, line_buffer + count, 1);

        if (line_buffer[count] == '\n') {
            line_buffer[count] = '\0';
            break;
        }
    }
    return 1;
}

int append_argv(char *line, char *argv[]) {
    int num = 0;
    char delimiter[2] = " ";
    char *ptr;

    ptr = strtok(line, delimiter);
    char *archive = ptr;
    while (ptr != NULL) {
        argv[num++] = ptr;
        ptr = strtok(NULL, delimiter);

        if (ptr == archive) {
            break;
        } else {
            archive = ptr;
        }
    }

    return num;
}

int main(int argc, char *argv[]) {
    int argv_count;
    char *line_buffer = (char *) malloc(sizeof(char) * BUFFER_SIZE);
    char **argv_buffer = (char **) malloc(sizeof(char *) * MAXARG);
    for (int i = 0; i < MAXARG; i++) {
        argv_buffer[i] = (char *) malloc(sizeof(char));
    }

    while (get_line(line_buffer)) {
        for (int i = 0; i < MAXARG; i++) {
            for (int j = 0; j < BUFFER_SIZE; j++) {
                argv_buffer[i][j] = 0;
            }
        }

        for (int i = 0; i < argc - 1; i++) {
            argv_buffer[i] = argv[i + 1];
        }

        argv_count = append_argv(line_buffer, argv_buffer + argc - 1);

        if (argv_count) {
            int pid = fork();
            if (pid == 0) {
                /* int sub process */
                exec(argv_buffer[0], argv_buffer);
            } else {
                wait(0);
            }
        }
        line_buffer = (char *) malloc(sizeof(char) * BUFFER_SIZE);
    }

    exit(0);
}