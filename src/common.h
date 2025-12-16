#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"

#define MAX_BUF 4096
#define MAX_BACKUPS 50
#define MAX_WATCHES 8192
#define EVENT_BUF_LEN (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define MAX_ARG 20

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define PERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__))

enum types
{
    FILE_TYPE,       // 0
    DIRECTORY_TYPE,  // 1
    SYMLINK_TYPE     // 2
};

typedef struct
{
    char* name;
    char* real_path;
    mode_t perms;
    enum types type;
} file;

struct backup
{
    int pid;
    char* source;
    char* target;
};

struct BackupsMap
{
    struct backup backup_map[MAX_BACKUPS];
    int backup_count;
};

void log_print(int ld, int pid, char* first, char* second, char* third);
file** scan_dir(char* path);
int isEmpty(char* target);
void add_to_BackupMap(struct BackupsMap* map, int pid, char* source, char* target);
struct backup* find_backup(struct BackupsMap* map, char* target);
void remove_from_BackupMap(struct BackupsMap* map, int pid);
void ignore_signals();
void free_file_pointer_tab(file** tab);
void sethandler(void (*f)(int), int sigNo);
void cleanup(char* input, char** argv, int argc, struct BackupsMap* map);
void usage(char* pname, char* args);
char* join_path(char* dir, char* name);
ssize_t bulk_write(int fd, char* buf, size_t count);
ssize_t bulk_read(int fd, char* buf, size_t count);
file* make_file_struct(char* name, char* path);

#endif
