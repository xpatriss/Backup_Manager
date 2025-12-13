#include "common.h"
#include "add_command.h"

// zwraca argc, uzupełnia tablice argv [instruction, paths]
int parse(char* input, char** argv)
{
    int argc = 0;

    // POMIJANIE SPACJI Z POCZĄTKU
    char* idx_start = input;
    while(memcmp(idx_start, " ", 1) == 0)
    {
        idx_start++;
    }
    // idx_start wskazuje na pierwszy znak nie będący spacją


    // POBIERANIE INSTRUKCJI
    char* idx_end_instr = strchr(idx_start, ' '); // szukanie pierwszej spacji
    if (idx_end_instr == NULL)                    // jeśli brak spacji to szukaj końca (ENTER)
    {
        idx_end_instr = strchr(idx_start, '\n');
    }
    // idx_end_instr wskazuje na spacje po instrukcji lub koniec linii jeśli nie ma spacji
    int instr_len = idx_end_instr - idx_start + 1;  
    char instruction[instr_len];
    strncpy(instruction, idx_start, (instr_len - 1));
    instruction[(instr_len - 1)] = '\0';

    argv[argc] = strdup(instruction);
    argc++;
    

    // ODCZYTYWANIE ŚCIEŻEK - bez obsługi ""

    if (*idx_end_instr != '\n')
    {
        // source path
        char* path_start = idx_end_instr + 1;         // początek ścieżki
        char* path_end = strchr(path_start, ' ');   // spacja po ścieżce
        if (path_end == NULL)                    // jeśli brak spacji to szukaj końca (ENTER)
        {
            path_end = strchr(path_start, '\n');
        }
        int path_length = path_end - path_start + 1;    // długość ścieżki
        char source_path[path_length];
        strncpy(source_path, path_start, (path_length - 1));
        source_path[(path_length - 1)] = '\0';
        argv[argc] = strdup(source_path);
        argc++;

        // target paths
        while((*path_end != '\n') && (*(path_end + 1) != ' ') && (*(path_end + 1) != '\n')) //zabezpiecznie przed spacjami na końcu
        {
            path_start = path_start + path_length;
            path_end = strchr(path_start, ' ');
            if (path_end == NULL)                    
            {
                path_end = strchr(path_start, '\n');
            }
            path_length = path_end - path_start + 1;
            char path[path_length];
            strncpy(path, path_start, (path_length - 1));
            path[(path_length - 1)] = '\0';
            argv[argc] = strdup(path);
            argc++;
        }
    }

    argv[argc] = NULL;

    // ROZPOZNAWANIE INSTRUKCJI
    int add = strcmp(argv[0], "add");
    int end = strcmp(argv[0], "end");
    int exit = strcmp(argv[0], "exit");
    int restore = strcmp(argv[0], "restore");
    int list = strcmp(argv[0], "list");


    if (add == 0)
    {
        //printf("\nAdd\n");
        if (argc < 3)
        {
            usage(argv[0], "<source path> <target path 1> <target path 2> ...");
            return 0;
        }
        errno = 0;
        char* source = realpath(argv[1], NULL);
        char* target = realpath(argv[2], NULL);
        if ((source == NULL) && (errno == ENOENT))
        {
            fprintf(stderr, "\nThe source directory doesn't exist!\n");
            return 0;
        }
        else if (source == NULL)
        {
            fprintf(stderr, "\n Source path is incorrect!\n");
            return 0;
        }
        if ((target == NULL) && (errno == ENOENT))
        {
            if(mkdir(argv[2], 0777))
            {
                perror("mkdir failed");
                return 0;
            }
            target = realpath(argv[2], NULL);
        }
        else if (target == NULL)
        {
            fprintf(stderr, "\n Target path is incorrect!\n");
            return 0;
        }
        free(argv[1]);
        argv[1] = source;
        free(argv[2]);
        argv[2] = target;
        
        add_action(argv[1], argv[2]);

    }
    else if (end == 0)
    {
        //printf("\nEnd\n");
        if (argc < 3)
        {
            usage(argv[0], "<source path> <target path 1> <target path 2> ...");
            return 0;
        }
    }
    else if (restore == 0)
    {
        //printf("\nRestore\n");
        if (argc != 3)
        {
            usage(argv[0], "<source path> <target path>");
            return 0;
        }
    }
    else if (exit == 0)
    {
        //printf("\nExit\n");
        if (argc != 1)
        {
            usage(argv[0], NULL);
            return 0;
        }
    }
    else if (list == 0)
    {
        //printf("\nList\n");
        if (argc != 1)
        {
            usage(argv[0], NULL);
            return 0;
        }
    }
    else
    {
        printf("\nUnrecognizable instruction!\n");
        return 0;
    }
    
    return argc;

}

