#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define BUFFER_SIZE     128
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

void append_argv(char *line, char *buffer[], int count) {
    int num = count;
    char delimiter[2] = " ";
    char *ptr;

    ptr = strtok(line, delimiter);
    char *archive = ptr;
    while (ptr != NULL) {
        buffer[num] = (char *) malloc(BUFFER_SIZE);
        strcpy(buffer[num], ptr);
        num++;

        ptr = strtok(NULL, delimiter);
        if (ptr == archive) {
            break;
        } else {
            archive = ptr;
        }
    }

    buffer[num] = 0;
}

int main(int argc, char *argv[]) {
    char line_buffer[BUFFER_SIZE];
    char *argv_buffer[BUFFER_SIZE];

    for (int i = 1; i < argc; i++) {
        argv_buffer[i - 1] = argv[i];
    }

    while (get_line(line_buffer)) {
        append_argv(line_buffer, argv_buffer, argc - 1);
        memset(line_buffer, 0, BUFFER_SIZE);

        if (fork() == 0) {
            exec(argv_buffer[0], argv_buffer);
        } else {
            wait(0);
        }
    }
    exit(0);
}