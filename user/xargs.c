#include "kernel/types.h"
#include "user/user.h"

#define BUFFER_SIZE     1024

int get_line(char *line_buffer) {
    int count = 0;
    while (scanf("%c", line_buffer + count) != '\n') {
        count += 1;
    }
}

void extend_args()

int main(int argc, char *argv[]) {
    char line_buffer[BUFFER_SIZE], count;

    while ((count = get_line(line_buffer)) > 0) {

    }
}