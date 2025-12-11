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

// przegląda katalog o podanej ścieżce, zapisuje nazwy elementów i ich rozmiary w tablicy
elements* scan_dir(char* path, elements result[3])
{
    DIR *dirp;  //wskaznik na strumień katalogu otwarty przez opendir()
    struct dirent *dp;  //zawiera inode i nazwe jednego elementu katalogu
    struct stat filestat;   //zawiera informacje o pliku (rozmiar, typ, dostęp)
     // Bufor na pełną ścieżkę

    int files_count = 0, dirs_count = 0, links_count = 0;

    elements* files = &result[0];
    elements* dirs = &result[1];
    elements* links = &result[2];

    /*
    char *files_names[MAX_SIZE];
    ssize_t files_sizes[MAX_SIZE];

    char *dirs_names[MAX_SIZE];
    ssize_t dirs_sizes[MAX_SIZE];

    char *links_names[MAX_SIZE];
    ssize_t links_sizes[MAX_SIZE];
    */

    if ((dirp = opendir(path)) == NULL)  //próba otwarcia katalogu
    {
        ERR("opendir");
    }
    
    do
    {
        errno = 0;  //zerujemy żeby móc wykryć błąd readdir, jesli jakas funkcja systemowa zwroci blad zmienia wartosc errno
        if ((dp = readdir(dirp)) != NULL)   //odczytuje kolejny element katalogu
        {
            // Pomijamy . i ..
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;

            char* full_path = join_path(path, dp->d_name);

            if (lstat(full_path, &filestat))   //sukces->0, błąd->-1
                ERR("lstat");
            // sprawdzanie typu elementu
            if (S_ISDIR(filestat.st_mode))
            {
                dirs->names[dirs_count] = strdup(dp->d_name);
                dirs->sizes[dirs_count] = filestat.st_size;
                dirs_count++;
            }
            else if (S_ISREG(filestat.st_mode))
            {
                files->names[files_count] = strdup(dp->d_name);
                files->sizes[files_count] = filestat.st_size;
                files_count++;
            }
            else if (S_ISLNK(filestat.st_mode))
            {        
                links->names[links_count] = strdup(dp->d_name);
                links->sizes[links_count] = filestat.st_size;
                links_count++;
            }
            else
            {    
                fprintf(stderr, "\nunrecognizable file type\n");
            }
            free(full_path);
        }
        
    } while (dp != NULL);    //pętla aż readdir zwróci NULL (koniec katalogu), errno=0 jeśli wszystko się powiodło

    if (errno != 0)     //jesli readdir zwrócił NULL ale errno != 0 -> wystąpił błąd
        ERR("readdir");
    if (closedir(dirp)) //próba zamknięcia katalogu
        ERR("closedir");

    files->count = files_count;
    dirs->count = dirs_count;
    links->count = links_count;
    return result;

    /*
    for (int i = 0; i < files; i++)
    {
        free(files_names[i]);
        free(files_sizes[i]);
    }
    for (int i = 0; i < dirs; i++)
    {
        free(dirs_names[i]);
        free(dirs_sizes[i]);
    }
    for (int i = 0; i < linkss; i++)
    {
        free(links_names[i]);
        free(links_sizes[i]);
    }
    */

        
}

void copy(char* source, char* target, elements content[3])
{
    // files - content[0]
    int files_count = content[0].count;
    for(int i = 0; i < files_count; i++)
    {
        int fd1, fd2;
        ssize_t n = content[0].sizes[i];
        char buf[4096];
        char* name = strdup(content[0].names[i]);
        char* source_name = join_path(source, name);
        char* target_name = join_path(target, name);

    
        if ((fd1 = TEMP_FAILURE_RETRY(open(source_name, O_RDONLY))) < 0)
        {
            ERR("open");
        }
        if ((fd2 = TEMP_FAILURE_RETRY(open(target_name, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0777))) < 0)
        {
            ERR("open");
        }
        int count = 0;
        ssize_t bytes_read;
        while((bytes_read = bulk_read(fd1, buf, sizeof(buf))) > 0)
        {
            if ((count = bulk_write(fd2, buf, bytes_read)) < 0)
                ERR("write");
        }

        if (bytes_read < 0) 
            ERR("read");


        if (TEMP_FAILURE_RETRY(close(fd1)))
            ERR("close");

        if (TEMP_FAILURE_RETRY(close(fd2)))
            ERR("close");

        free(source_name);
        free(target_name);
        free(name);

    }

    // dirs - content[1]

    // links - content[2]
}

void add_action(char* source, char* target)
{
    elements content[3];             // tablica struktur elementów
    scan_dir(source, content);      // skanuje zawartość katalogu źródłowego
    int status = mkdir(target, 0777);
    copy(source, target, content);         // kopiuje elementy do katalogu docelowego

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < content[i].count; j++)
        {
            free(content[i].names[j]);
            //free(content[i].sizes[j]);
        }
    }
}