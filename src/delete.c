#include "delete.h"

void delete_file(char* source, char* target, char* filepath)
{
    size_t src_len = strlen(source);
    char* newpath = filepath + src_len;
    if (*newpath == '/')
    {
        newpath++;
    }
    char* target_path = join_path(target, newpath);
    if ((remove(target_path)) != 0)
    {
        if (errno != ENOENT)
        {
            perror("remove failed");
        }
        free(target_path);
        return;
    }
    free(target_path);
    return;
}

int recursive_clean(char* path)
{
    DIR* d = opendir(path);
    if (!d)
    {
        perror("opendir failed");
        return -1;
    }

    struct dirent* p;
    while ((p = readdir(d)))
    {
        if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            continue;

        char* full_path = join_path(path, p->d_name);

        struct stat st;

        if (lstat(full_path, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                recursive_clean(full_path);
            }
            else
            {
                if (unlink(full_path) != 0)
                {
                    perror("Failed to delete file inside dir");
                }
            }
        }
        free(full_path);
    }

    closedir(d);

    if (rmdir(path) != 0)
    {
        perror("Failed to remove directory");
        return -1;
    }

    return 0;
}

void delete_dir(char* source, char* target, char* dirpath)
{
    size_t src_len = strlen(source);

    char* newpath = dirpath + src_len;
    if (*newpath == '/')
        newpath++;

    char* target_path = join_path(target, newpath);

    // printf("Recursively deleting dir from backup: %s\n", target_path);

    recursive_clean(target_path);

    free(target_path);
}
