#ifndef COMMON_H 
#define COMMON_H

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h> 
#include <sys/stat.h>  
#include <unistd.h> 
#include <ftw.h> 

#define MAX_ARG 10
#define MAX_SIZE 1000
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), kill(0, SIGKILL), exit(EXIT_FAILURE))

typedef struct
{
    char* names[MAX_SIZE];
    ssize_t sizes[MAX_SIZE];
    int count;
} elements;

void usage(char* pname, char* args);
char* join_path(char *dir, char *name);
//void safe_malloc(size_t size);

#endif 