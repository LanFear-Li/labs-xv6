struct stat;
struct rtcdate;

typedef void (*sigfunc_t)(int);
typedef void (*sigrestorer_t)();

#define WEXITSTATUS(status)     ((status) & 0xff)
#define WTERMSIG(status)        (((status) & 0xff00) >> 8)
#define WIFEXITED(status)       (WTERMSIG(status) == 0)
#define WIFSIGNALED(status)     (WTERMSIG(status) != 0)

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int, int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int sctty(int);
int sigaction(int, sigfunc_t, sigrestorer_t);
int sigreturn(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
char *strtok(char *str, char *filter);

// signal.S
void sigrestorer();
