#include "parse.h"

// zwraca argc, uzupełnia tablice argv [instruction, paths]
int parse(char* input, char** argv)
{
    int argc = 0;
    char* ptr = input;

    while (*ptr != '\0' && argc < MAX_ARG)
    {
        while (isspace((unsigned char)*ptr))
        {
            ptr++;
        }

        if (*ptr == '\0')
            break;

        char* start = NULL;
        int len = 0;

        if (*ptr == '"')
        {
            // TRYB CUDZYSŁOWU
            ptr++;
            start = ptr;

            while (*ptr != '\0' && *ptr != '"')
            {
                ptr++;
                len++;
            }

            if (*ptr == '\0')
            {
                fprintf(stderr, ANSI_COLOR_RED "\nInvalid path! (Missing closing \")\n" ANSI_COLOR_RESET);

                for (int i = 0; i < argc; i++)
                {
                    free(argv[i]);
                    argv[i] = NULL;
                }
                return 0;
            }

            ptr++;
        }
        else
        {
            // TRYB NORMALNY (bez cudzysłowu)
            start = ptr;

            while (*ptr != '\0' && !isspace((unsigned char)*ptr))
            {
                ptr++;
                len++;
            }
        }

        argv[argc] = malloc(len + 1);
        if (argv[argc] == NULL)
        {
            ERR("malloc");
            return 0;
        }

        strncpy(argv[argc], start, len);
        (argv[argc])[len] = '\0';

        argc++;
    }

    for (int i = argc; i < MAX_ARG; i++)
    {
        argv[i] = NULL;
    }

    return argc;
}
