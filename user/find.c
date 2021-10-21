#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define BUFFER_SIZE 512
#define NULL        0

int file_validate(int *fd, char *path, struct stat *state) {
    if ((*fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        return 1;
    } else if (fstat(*fd, state) < 0) {
        printf("find: cannot stat %s\n", path);
        close(*fd);
        return 1;
    }
    return 0;
}

void find(char *path, char *name) {
    int fd;
    char buffer[BUFFER_SIZE], *ptr;
    /* stat provides detailed info about a file */
    struct stat state;
    /* get file entry of a directory, including serial number and entry name */
    struct dirent dir_entry;

    if (file_validate(&fd, path, &state)) {
        exit(1);
    }

    /* add layer */
    strcpy(buffer, path);
    ptr = buffer + strlen(buffer);
    *ptr++ = '/';

    while (read(fd, &dir_entry, sizeof(dir_entry)) == sizeof(dir_entry)) {
        if (dir_entry.inum == 0) {
            continue;
        }

        memmove(ptr, dir_entry.name, DIRSIZ);
        ptr[DIRSIZ] = NULL;
        if (stat(buffer, &state) < 0) {
            printf("ls: cannot stat %s\n", buffer);
            continue;
        }

        switch (state.type) {
            case T_FILE:
                if (strcmp(dir_entry.name, name) == 0) {
                    printf("%s\n", buffer);
                }
                break;
            case T_DIR:
                /* ignore . and .. directories */
                if (strcmp(dir_entry.name, ".") && strcmp(dir_entry.name, "..")) {
                    find(buffer, name);
                }
                break;
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("lack in parameters.\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}