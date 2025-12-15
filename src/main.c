#include "add_command.h"
#include "common.h"
#include "parse.h"

volatile sig_atomic_t app_running = 1;

void exit_handler(int sig)
{
    app_running = 0;
}

void making_realpaths(int instr_argc, char** instr_argv)
{
    errno = 0;
            char* source = realpath(instr_argv[1], NULL);
            if ((source == NULL) && (errno == ENOENT))
            {
                fprintf(stderr, ANSI_COLOR_RED "\nThe source directory doesn't exist!\n" ANSI_COLOR_RESET);
                free(source);
                return;
            }
            else if (source == NULL)
            {
                perror("realpath failed");
                free(source);
                return;
            }

            char* targets[instr_argc - 2]; 
            for (int i = 0; i < (instr_argc - 2); i++)
            {
                errno = 0;
                targets[i] = realpath(instr_argv[i + 2], NULL);
                if ((targets[i] == NULL) && (errno == ENOENT))
                {
                    if (mkdir(instr_argv[i + 2], 0777))
                    {
                        perror("mkdir failed");
                        for (int j = 0; j < (instr_argc - 2); j++)
                        {
                            if (targets[j] != NULL) {
                                free(targets[j]);
                            }
                        }
                        return;
                    }
                    targets[i] = realpath(instr_argv[i + 2], NULL);
                }
                else if (targets[i] == NULL)
                {
                    perror("realpath failed");
                    for (int j = 0; j < (instr_argc - 2); j++)
                    {
                        if (targets[j] != NULL) {
                            free(targets[j]);
                        }
                    }
                    return;
                }
            }

            free(instr_argv[1]);
            instr_argv[1] = source;
            for (int i = 2; i < instr_argc; i++)
            {
                free(instr_argv[i]);
                instr_argv[i] = targets[i - 2];
            }
}

void get_instruction(int instr_argc, char** instr_argv)
{
        if ((strcmp(instr_argv[0], "add")) == 0)
        {
            if (instr_argc < 3)
            {
                usage(instr_argv[0], "<source path> <target path 1> <target path 2> ...");
                return;
            }

            making_realpaths(instr_argc, instr_argv);
            char* source = instr_argv[1];
            size_t src_len = strlen(source);

            for (int i = 2; i < instr_argc; i++)
            {
                char* target = instr_argv[i];
                if(strncmp(source, target, src_len) == 0)
                {
                    if (target[src_len] == '/' || target[src_len] == '\0')
                    {
                        fprintf(stderr, ANSI_COLOR_RED "\nTarget '%s' is inside source '%s'!\n" ANSI_COLOR_RESET, target, source);
                        return;
                    }
                }
            }
            int ret = fork();
            switch (ret)
            {
                case(-1):
                    fprintf(stderr, ANSI_COLOR_RED "\nFork failed!\n" ANSI_COLOR_RESET);
                    return;
                case(0):
                    add_action(instr_argv[1], instr_argv[2]);
                    exit(EXIT_SUCCESS);
                    break;
                default:
                    break;

            }
        }
        else if ((strcmp(instr_argv[0], "end")) == 0)
        {
            if (instr_argc < 3)
            {
                usage(instr_argv[0], "<source path> <target path 1> <target path 2> ...");
                return;
            }
        }
        else if ((strcmp(instr_argv[0], "restore")) == 0)
        {
            if (instr_argc != 3)
            {
                usage(instr_argv[0], "<source path> <target path>");
                return;
            }
        }
        else if ((strcmp(instr_argv[0], "exit") == 0))
        {
            if (instr_argc != 1)
            {
                usage(instr_argv[0], NULL);
                return;
            }

            app_running = 0;
        }
        else if ((strcmp(instr_argv[0], "list")) == 0)
        {
            if (instr_argc != 1)
            {
                usage(instr_argv[0], NULL);
                return;
            }
        }
        else
        {
            printf(ANSI_COLOR_RED "\nUnrecognizable instruction!\n" ANSI_COLOR_RESET);
            return;
        }
}

int main(int argc, char** argv)
{
    if (argc != 1)
    {
        usage(argv[0], NULL);
        exit(EXIT_FAILURE);
    }

    sethandler(exit_handler, SIGINT);
    sethandler(exit_handler, SIGTERM);

    sigset_t mask;
    
    sigfillset(&mask);

    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTERM);

    sigdelset(&mask, SIGSEGV); // Błąd pamięci
    sigdelset(&mask, SIGFPE);  // Dzielenie przez zero itp.
    sigdelset(&mask, SIGILL);  // Nieprawidłowa instrukcja
    sigdelset(&mask, SIGBUS);  // Błąd szyny

    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("sigprocmask error");
        exit(EXIT_FAILURE);
    }

    printf(ANSI_BOLD "\nHello! Welcome to the Backup Manager. You can use one of the commands:\n" ANSI_COLOR_RESET);
    printf("\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "add" ANSI_COLOR_RESET
                                        " <source> <target>       - starts making a copy of source in target (you can "
                                        "add many target paths divided by spaces)\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "end" ANSI_COLOR_RESET
                                        " <source> <target>       - breaks making a copy of source in target (you can "
                                        "add many target paths divided by spaces)\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "restore" ANSI_COLOR_RESET
                                        " <source> <target>   - restore a copy of source from target\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "list" ANSI_COLOR_RESET
                                        "                        - shows a summary of folders with backups\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "exit" ANSI_COLOR_RESET
                                        "                        - stops running the program\n");
    printf("\n");

    size_t len = 0;
    char* input = NULL;
    char* instr_argv[MAX_ARG] = {0};
    int instr_argc = 0;

    while (app_running)
    {
        printf("\nWrite a command and press ENTER\n");
        if (getline(&input, &len, stdin) == -1)
        {
            if (errno == EINTR) continue; 
            break;
        }

        if (strcmp(input, "\n") == 0)
        {
            continue;
        }

        memset(instr_argv, 0, sizeof(instr_argv));

        instr_argc = parse(input, instr_argv);

        
        

        if (instr_argc != 0)
        {
            get_instruction(instr_argc, instr_argv);
        }

        // printf("\nDEBUG: argc = %d, instruction = %s\n", instr_argc, instr_argv[0]);
        /*
        if (instr_argc > 1)
            printf(" source = %s\n", instr_argv[1]);
        if (instr_argc > 2)
            printf(" target = %s\n", instr_argv[2]);
        */

        for (int i = 0; i < instr_argc; i++)
        {
            free(instr_argv[i]);
            instr_argv[i] = NULL;
        }
    }

    cleanup(input, instr_argv, 0);
    while (wait(NULL) > 0)
    {
    }
    return 0;
}
