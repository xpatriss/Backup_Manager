#include "copy.h"

void copy_file(char* target, file* f)
{
    int fd1, fd2;
    char buf[MAX_BUF];
    char* source_path = strdup(f->real_path);
    char* target_path = join_path(target, f->name);

    if ((fd1 = TEMP_FAILURE_RETRY(open(source_path, O_RDONLY))) < 0)
    {
        PERR("open");
        free(source_path);
        free(target_path);
        return;
    }
    if ((fd2 = TEMP_FAILURE_RETRY(open(target_path, O_WRONLY | O_CREAT | O_TRUNC, 0600))) < 0)
    {
        PERR("open");
        close(fd1);
        free(source_path);
        free(target_path);
        return;
    }
    if (fchmod(fd2, f->perms) == -1)
    {
        PERR("fchmod");
        close(fd2);
        free(source_path);
        free(target_path);
        return;
    }

    int count = 0;
    ssize_t bytes_read;
    free(source_path);
    free(target_path);

    while ((bytes_read = bulk_read(fd1, buf, sizeof(buf))) > 0)
    {
        if ((count = bulk_write(fd2, buf, bytes_read)) < 0)
        {
            PERR("write");
            return;
        }
    }
    if (bytes_read < 0)
    {
        PERR("read");
        return;
    }

    if (TEMP_FAILURE_RETRY(close(fd1)))
    {
        PERR("close");
        return;
    }

    if (TEMP_FAILURE_RETRY(close(fd2)))
    {
        PERR("close");
        return;
    }
}

void copy_symlink(char* s_root, char* t_root, char* source, char* target, file* s)
{
    char link_content[PATH_MAX];
    ssize_t len;

    len = readlink(s->real_path, link_content, sizeof(link_content) - 1);

    if (len == -1)
    {
        PERR("readlink");
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
        PERR("symlink");
        free(new_name);
        free(new_content);
        return;
    }

    free(new_name);
    free(new_content);
}

void copy_directory(char* s_root, char* t_root, char* target, file* d)
{
    file** dir_content = scan_dir(d->real_path);

    char* new_dir = join_path(target, d->name);

    if (mkdir(new_dir, 0700))
    {
        if (errno != EEXIST)
        {
            PERR("mkdir");
        }
        free_file_pointer_tab(dir_content);
        free(new_dir);
        return;
    }

    if (chmod(new_dir, d->perms) == -1)
    {
        PERR("chmod");
        free_file_pointer_tab(dir_content);
        free(new_dir);
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
