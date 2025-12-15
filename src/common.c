#include "common.h"

void usage(char* pname, char* args)
{
    if (args != NULL)
        fprintf(stderr, ANSI_COLOR_CYAN "USAGE: %s %s\n" ANSI_COLOR_RESET, pname, args);
    else
        fprintf(stderr, ANSI_COLOR_CYAN "USAGE: %s \n" ANSI_COLOR_RESET, pname);
    // exit(EXIT_FAILURE);
}

char* join_path(char* dir, char* name)
{
    size_t len = strlen(dir) + strlen(name) + 2;

    char* result = malloc(len);
    if (result == NULL)
    {
        ERR("malloc");
    }

    snprintf(result, len, "%s/%s", dir, name);

    return result;
}

void add_to_BackupMap(struct BackupsMap *map, int pid, char* source, char* target) 
{
    if (map->backup_count >= MAX_BACKUPS) {
        fprintf(stderr, "Exceeded max backups!\n");
        return;
    }
    
    map->backup_map[map->backup_count].pid = pid;
    map->backup_map[map->backup_count].source = strdup(source);
    map->backup_map[map->backup_count].target = strdup(target);
    map->backup_count++;
    printf("new backup of '%s': '%s' pid=%d\n", source, target, pid);
}

struct backup *find_backup(struct BackupsMap *map, int pid) 
{
    for (int i = 0; i < map->backup_count; i++) {
        if (map->backup_map[i].pid == pid) {
            return &map->backup_map[i];
        }
    }
    return NULL;
}

void remove_from_BackupMap(struct BackupsMap *map, int pid) 
{
    for (int i = 0; i < map->backup_count; i++) {
        if (map->backup_map[i].pid == pid) 
        {
            printf("removing backup of '%s': '%s' pid=%d\n", map->backup_map[i].source, map->backup_map[i].target, pid);
            //printf("removing backup: '%s' @pid=%d\n", map->backup_map[i].path, pid);
            free(map->backup_map[i].source);
            free(map->backup_map[i].target);
            map->backup_map[i] = map->backup_map[map->backup_count - 1];
            map->backup_count--;
            return;
        }
    }
}

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


int isEmpty(char* target)
{
    file** target_content = scan_dir(target);
        if (target_content[0] != NULL)
        {
            fprintf(stderr, ANSI_COLOR_RED "\nTarget: '%s' directory in not empty!\n" ANSI_COLOR_RESET, target);
            free_file_pointer_tab(target_content);
            
            return -1;
        }
    free_file_pointer_tab(target_content);
    return 0;
}

void cleanup(char* input, char** argv, int argc) {
    printf(ANSI_BOLD ANSI_COLOR_YELLOW "\nCleaning up resources...\n" ANSI_COLOR_RESET);

    if (input != NULL) {
        free(input);
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i] != NULL) 
            free(argv[i]);
    }

    // 3. ZABICIE PROCESÓW POTOMNYCH (Wymaganie projektowe)
    // kill(0, SIGTERM) wysyła sygnał do całej grupy procesów (czyli do rodzica i wszystkich dzieci)
    // Ale musimy uważać, żeby rodzic nie zabił sam siebie zanim nie skończy funkcji.
    // Dlatego w tym prostym modelu zakładamy, że dzieci mają swoje grupy lub po prostu:
    
    /*
    // Opcja bezpieczna: Czekamy na wszystkie dzieci (zombie cleanup)
    while (waitpid(-1, NULL, WNOHANG) > 0); 
    
    // Opcja "atomowa" (zgodna z poleceniem "zakończyć pracę procesów"):
    // Wysyłamy sygnał do wszystkich procesów w grupie, ignorując go u siebie na chwilę
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM); 
    
    // Czekamy aż dzieci umrą
    while(wait(NULL) > 0);
    */
    printf(ANSI_BOLD ANSI_COLOR_YELLOW "END\n" ANSI_COLOR_RESET);
}

void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;

    if (-1 == sigaction(sigNo, &act, NULL))
        return;
}

void free_file_pointer_tab(file** tab)
{
    int i = 0;
            while (tab[i] != NULL)
            {
                free(tab[i]->name);
                free(tab[i]->real_path);
                free(tab[i]);
                i++;
            }
            free(tab);
}

ssize_t bulk_read(int fd, char* buf, size_t count)
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

ssize_t bulk_write(int fd, char* buf, size_t count)
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

file* make_file_struct(char* name, char* path)
{
    struct stat file_info;

                    file* element = malloc(sizeof(file));

                    if (element == NULL)
                    {
                        ERR("malloc");
                    }
                        
                    if (lstat(path, &file_info) == 0) 
                    {
                        element->name = strdup(name);
                        element->real_path = strdup(path);
                        element->perms = file_info.st_mode & 0777;
                        if (S_ISREG(file_info.st_mode))
                        {
                            element->type = FILE_TYPE;
                        }
                        else if (S_ISDIR(file_info.st_mode))
                        {
                            element->type = DIRECTORY_TYPE;
                        }
                        else if (S_ISLNK(file_info.st_mode))
                        {
                            element->type = SYMLINK_TYPE;
                        }
                        else
                        {
                            fprintf(stderr, "\nunrecognizable file type\n");
                            return NULL;
                        }
                    }
                    else
                    {
                        perror("lstat");
                    }

                    return element;


}

void ignore_signals()
{
    for (int i = 1; i < NSIG; i++) 
    {
        
        if (i == SIGINT || i == SIGTERM || i == SIGKILL || i == SIGSTOP) 
        {
            continue;
        }

        sethandler(SIG_IGN, i);
    }
}

/*
void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        ERR("malloc");
    }
    return ptr;
}
*/
