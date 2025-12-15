#ifndef COMMON_H 
#define COMMON_H

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h> 
#include <sys/stat.h>  
#include <unistd.h> 
#include <ftw.h> 
#include <ctype.h>
#include <sys/inotify.h>
#include <limits.h>

// Definicje kolorów ANSI
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
// Style
#define ANSI_BOLD          "\x1b[1m"

#define MAX_WATCHES 8192
#define EVENT_BUF_LEN (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define MAX_ARG 10
#define MAX_SIZE 4096
#define MAX_PATH 4096

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))


enum types
{
    FILE_TYPE,       // 0
    DIRECTORY_TYPE,  // 1
    SYMLINK_TYPE     // 2
};

typedef struct 
{
    char* name;//[MAX_SIZE];
    char* real_path;//[MAX_SIZE];
    //ssize_t size;
    mode_t perms;
    enum types type;
} file;

void free_file_pointer_tab(file** tab);
void sethandler(void (*f)(int), int sigNo);
void cleanup(char* input, char** argv, int argc);
void usage(char* pname, char* args);
char* join_path(char *dir, char *name);
ssize_t bulk_write(int fd, char *buf, size_t count);
ssize_t bulk_read(int fd, char *buf, size_t count);
file* make_file_struct(char* name, char* path);
//void safe_malloc(size_t size);

#endif 