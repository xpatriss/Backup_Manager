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
        ERR("sigaction");
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

/*
void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        ERR("malloc");
    }
    return ptr;
}
*/
