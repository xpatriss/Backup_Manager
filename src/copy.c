#include "copy.h"

file** scan_dir(char* path)
{
    DIR* dirp;             
    struct dirent* dp;     
    struct stat filestat;  

    file** tab = NULL;
    int content_count = 0;

    if ((dirp = opendir(path)) == NULL)
    {
        perror("opendir failed");
        return NULL;
    }

    do
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;

            char* full_path = join_path(path, dp->d_name);

            if (lstat(full_path, &filestat))
            {
                free(full_path);
                perror("lstat failed");
                return NULL;
            }

            if (!S_ISDIR(filestat.st_mode) && !S_ISREG(filestat.st_mode) && !S_ISLNK(filestat.st_mode))
            {
                fprintf(stderr, "\nunrecognizable file type\n");
                continue;
            }

            ssize_t new_size = (content_count + 2) * sizeof(file*);
                file** temp = realloc(tab, new_size);

                if (temp == NULL)
                {
                    ERR("realloc");
                }

                tab = temp;

                tab[content_count] = malloc(sizeof(file));
                tab[content_count]->name = strdup(dp->d_name);
                tab[content_count]->real_path = strdup(full_path);
                tab[content_count]->perms = filestat.st_mode & 0777;

            if (S_ISDIR(filestat.st_mode))
            {
                tab[content_count]->type = DIRECTORY_TYPE;
            }
            else if (S_ISREG(filestat.st_mode))
            {
                tab[content_count]->type = FILE_TYPE;
            }
            else if (S_ISLNK(filestat.st_mode))
            {
                tab[content_count]->type = SYMLINK_TYPE;
            }
            
            content_count++;
            free(full_path);
        }

    } while (dp != NULL);

    if (errno != 0)
    {
        perror("readdir failed");
        return NULL;
    }

    if (closedir(dirp))
    {
        perror("closedir failed");
        return NULL;
    }

    if (content_count == 0)
    {
        ssize_t new_size = (content_count + 1) * sizeof(file*);
        file** temp = realloc(tab, new_size);

        if (temp == NULL)
        {
            ERR("realloc");
        }

        tab = temp;
    }
    tab[content_count] = NULL;

    return tab;
}




void copy_file(char* target, file* f)
{
    int fd1, fd2;
    char buf[4096];
    char* source_path = strdup(f->real_path);
    char* target_path = join_path(target, f->name);

    if ((fd1 = TEMP_FAILURE_RETRY(open(source_path, O_RDONLY))) < 0)
    {
        perror("open file failed");
        return;
    }
    if ((fd2 = TEMP_FAILURE_RETRY(open(target_path, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, f->perms))) < 0)
    {
        perror("open file failed");
        return;
    }

    int count = 0;
    ssize_t bytes_read;
    while ((bytes_read = bulk_read(fd1, buf, sizeof(buf))) > 0)
    {
        if ((count = bulk_write(fd2, buf, bytes_read)) < 0)
        {
            perror("write to file failed");
            return;
        }
    }
    if (bytes_read < 0)
    {
        perror("read a file failed");
        return;
    }

    if (TEMP_FAILURE_RETRY(close(fd1)))
    {
        perror("close a file failed");
        return;
    }

    if (TEMP_FAILURE_RETRY(close(fd2)))
    {
        perror("close a file failed");
        return;
    }

    free(source_path);
    free(target_path);
}

void copy_symlink(char* s_root, char* t_root, char* source, char* target, file* s)
{
    char link_content[MAX_PATH];
    ssize_t len;

    len = readlink(s->real_path, link_content, sizeof(link_content) - 1);

    if (len == -1)
    {
        perror("readlink");
        return;
    }

    link_content[len] = '\0';
    char* new_content = strdup(link_content);     // sciezka do pliku
    char* new_name = join_path(target, s->name);  // nazwa symlinku

    ssize_t source_len = strlen(s_root);
    if (strncmp(link_content, s_root, source_len) == 0)
    {
        char* file_name = link_content + source_len + 1;  // nazwa wskazywanego pliku
        char* modified = join_path(t_root, file_name);    // nowa zawarosc symlnika
        free(new_content);
        new_content = strdup(modified);

        free(modified);
    }

    unlink(new_name);

    if (symlink(new_content, new_name) == -1)
    {
        perror("creating a symlink failed");
        return;
    }

    free(new_name);
    free(new_content);
}

void copy_directory(char* s_root, char* t_root, char* target, file* d)
{
    file** dir_content = scan_dir(d->real_path);

    char* new_dir = join_path(target, d->name);

    if (mkdir(new_dir, 0777))
    {
        perror("mkdir failed");
        free(dir_content);
        free(new_dir);
        return;
    }

    if (dir_content == NULL)
        return;

    int i = 0;

    while (dir_content[i] != NULL)
    {
        switch (dir_content[i]->type)
        {
            case (FILE_TYPE):
                copy_file(new_dir, dir_content[i]);
                break;
            case (DIRECTORY_TYPE):
                copy_directory(s_root, t_root, new_dir, dir_content[i]);
                break;
            case (SYMLINK_TYPE):
                copy_symlink(s_root, t_root, d->real_path, new_dir, dir_content[i]);
                break;
            default:
                continue;
        }
        i++;
    }

    free_file_pointer_tab(dir_content);
    free(new_dir);
}

void copy(char* s_root, char* t_root, char* source, char* target, file** content)
{
    if (content == NULL)
        return;
    int i = 0;
    while (content[i] != NULL)
    {
        switch (content[i]->type)
        {
            case (FILE_TYPE):
                copy_file(target, content[i]);
                break;
            case (DIRECTORY_TYPE):
                copy_directory(s_root, t_root, target, content[i]);
                break;
            case (SYMLINK_TYPE):
                copy_symlink(s_root, t_root, source, target, content[i]);
                break;
            default:
                continue;
        }
        i++;
    }
}
