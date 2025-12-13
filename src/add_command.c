#include "common.h"

ssize_t bulk_read(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;

    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0)
            return c;

        if (c == 0)
            return len;  // EOF

        buf += c;
        len += c;
        count -= c;
    } while (count > 0);

    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;

    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;

        buf += c;
        len += c;
        count -= c;
    } while (count > 0);

    return len;
}

file** scan_dir(char* path)
{
    DIR *dirp;  //wskaznik na strumień katalogu otwarty przez opendir()
    struct dirent *dp;  //zawiera inode i nazwe jednego elementu katalogu
    struct stat filestat;   //zawiera informacje o pliku (rozmiar, typ, dostęp)

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
                
            
            if (S_ISDIR(filestat.st_mode))
            {
                ssize_t new_size = (content_count + 2) * sizeof(file*);
                file** temp = realloc(tab, new_size);

                if (temp == NULL) {
                    ERR("realloc");
                }

                tab = temp;
            

                tab[content_count] = malloc(sizeof(file));
                tab[content_count]->name = strdup(dp->d_name);
                tab[content_count]->real_path = strdup(full_path);
                tab[content_count]->perms = filestat.st_mode & 0777;
                tab[content_count]->type = DIRECTORY_TYPE;
                
                content_count++;
            }
            else if (S_ISREG(filestat.st_mode))
            {
                ssize_t new_size = (content_count + 2) * sizeof(file*);
                file** temp = realloc(tab, new_size);

                if (temp == NULL) {
                    ERR("realloc");
                }

                tab = temp;
            

                tab[content_count] = malloc(sizeof(file));
                tab[content_count]->name = strdup(dp->d_name);
                tab[content_count]->real_path = strdup(full_path);
                tab[content_count]->perms = filestat.st_mode & 0777;
                tab[content_count]->type = FILE_TYPE;
                
                content_count++;
            }
            else if (S_ISLNK(filestat.st_mode))
            {        
                ssize_t new_size = (content_count + 2) * sizeof(file*);
                file** temp = realloc(tab, new_size);

                if (temp == NULL) {
                    ERR("realloc");
                }

                tab = temp;
            

                tab[content_count] = malloc(sizeof(file));
                tab[content_count]->name = strdup(dp->d_name);
                tab[content_count]->real_path = strdup(full_path);
                tab[content_count]->perms = filestat.st_mode & 0777;
                tab[content_count]->type = SYMLINK_TYPE;
                
                content_count++;
            }
            else
            {    
                fprintf(stderr, "\nunrecognizable file type\n");
            }
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

                if (temp == NULL) {
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
    char* target_path = join_path(target,f->name);
    
    
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
        while((bytes_read = bulk_read(fd1, buf, sizeof(buf))) > 0)
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
    
    
    if (len == -1) {
        perror("readlink");
        return;
    }

    link_content[len] = '\0';
    char* new_content = strdup(link_content);   // sciezka do pliku
    char* new_name = join_path(target, s->name);   // nazwa symlinku

    ssize_t source_len = strlen(s_root);
    if (strncmp(link_content, s_root, source_len) == 0)
    {
        char* file_name = link_content + source_len + 1; // nazwa wskazywanego pliku
        char* modified = join_path(t_root, file_name);  // nowa zawarosc symlnika
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

void copy(char* s_root, char* t_root, char* source, char* target, file** content)
{
    if (content == NULL) return;
    int i = 0;
    while (content[i] != NULL)
    {
        switch(content[i]->type)
        {
            case(FILE_TYPE):
                copy_file(target, content[i]);
                break;
            case(DIRECTORY_TYPE):
                file** dir_content = scan_dir(content[i]->real_path);
                char* new_dir = join_path(target, content[i]->name);
                errno = 0;
                DIR* dirp;
                if ((dirp = opendir(new_dir)) == NULL)  
                {
                    if (errno == ENOENT)
                    {
                        if(mkdir(new_dir, 0777))
                        {
                            perror("mkdir failed");
                            return;
                        }
                    }
                    else
                    {
                        perror("opendir failed");
                        return;
                    }
                }
                else
                {
                    file** target_content = scan_dir(target);
                    if (target_content[0] != NULL)
                    {
                        fprintf(stderr, "\nTarget directory in not empty!\n");
                        return;
                    }
                    int j = 0;
                        while (target_content[j] != NULL)
                        {
                            free(target_content[j]->name);
                            free(target_content[j]->real_path);
                            free(target_content[j]);
                            j++;
                        }
                        free(target_content);
        
                    if (closedir(dirp)) 
                    {
                        perror("closedir failed");
                        return;
                    }

                }
                copy(s_root, t_root, content[i]->real_path, new_dir, dir_content);
                int j = 0;
                while (dir_content[j] != NULL)
                {
                    free(dir_content[j]->name);
                    free(dir_content[j]->real_path);
                    free(dir_content[j]);
                    j++;
                }
                free(dir_content);  
                free(new_dir)   ;
                break;
            case(SYMLINK_TYPE):
                copy_symlink(s_root, t_root, source, target, content[i]);
                break;
            default:
                continue;
        }
        i++;
    }
    
}


void add_action(char* source, char* target)
{
    
    /*if (content == NULL)
    {
        fprintf(stderr, "\nEmpty directory!\n");
        return;
    }*/
    errno = 0;
    DIR* dirp;
    if ((dirp = opendir(target)) == NULL)  
    {
        if (errno == ENOENT)
        {
            if(mkdir(target, 0777))
            {
                perror("mkdir failed");
                return;
            }
        }
        else
        {
            perror("opendir failed");
            return;
        }
    }
    else
    {
        file** target_content = scan_dir(target);
        if (target_content[0] != NULL)
        {
            fprintf(stderr, "\nTarget directory in not empty!\n");
            return;
        }
        int j = 0;
            while (target_content[j] != NULL)
            {
                free(target_content[j]->name);
                free(target_content[j]->real_path);
                free(target_content[j]);
                j++;
            }
            free(target_content);
        
        if (closedir(dirp)) 
        {
            perror("closedir failed");
            return;
        }

    }
    file** content = scan_dir(source);
    
    copy(source, target, source, target, content);         // kopiuje elementy do katalogu docelowego

    int i = 0;
    while (content[i] != NULL)
    {
        free(content[i]->name);
        free(content[i]->real_path);
        free(content[i]);
        i++;
    }
    free(content);
    
}