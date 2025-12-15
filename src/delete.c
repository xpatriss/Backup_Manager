#include "delete.h"

void delete_file(char* source, char* target, char* filepath)
{
    int len = strlen(source);
    char* newpath = filepath + len + 1;
    char* target_path = join_path(target, newpath);
    if((remove(target_path)) != 0)
    {
        perror("remove failed");
        free(target_path);
        return;
    }
    free(target_path);
    return;
}