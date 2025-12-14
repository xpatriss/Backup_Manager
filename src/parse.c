#include "add_command.h"
#include "common.h"

// zwraca argc, uzupełnia tablice argv [instruction, paths]
int parse(char* input, char** argv)
{
    int argc = 0;

    // POMIJANIE SPACJI Z POCZĄTKU
    char* idx_start = input;
    while (memcmp(idx_start, " ", 1) == 0)
    {
        idx_start++;
    }
    // idx_start wskazuje na pierwszy znak nie będący spacją

    // POBIERANIE INSTRUKCJI
    char* idx_end_instr = strchr(idx_start, ' ');  // szukanie pierwszej spacji
    if (idx_end_instr == NULL)                     // jeśli brak spacji to szukaj końca (ENTER)
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
        char* path_start = idx_end_instr + 1;      // początek ścieżki
        char* path_end = strchr(path_start, ' ');  // spacja po ścieżce
        if (path_end == NULL)                      // jeśli brak spacji to szukaj końca (ENTER)
        {
            path_end = strchr(path_start, '\n');
        }
        int path_length = path_end - path_start + 1;  // długość ścieżki
        char source_path[path_length];
        strncpy(source_path, path_start, (path_length - 1));
        source_path[(path_length - 1)] = '\0';
        argv[argc] = strdup(source_path);
        argc++;

        // target paths
        while ((*path_end != '\n') && (*(path_end + 1) != ' ') &&
               (*(path_end + 1) != '\n'))  // zabezpiecznie przed spacjami na końcu
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


    return argc;
}
