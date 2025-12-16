#include "add_command.h"
#include "common.h"
#include "end_command.h"
#include "list_command.h"
#include "parse.h"
#include "restore.h"

volatile sig_atomic_t app_running = 1;
volatile sig_atomic_t backup_crashed = 0;

void handle_exit_signal(int sig) { app_running = 0; }

void handle_backup_crashed(int sig) { backup_crashed = 1; }

void exit_function(struct BackupsMap* map) { app_running = 0; }

int making_realpaths(int instr_argc, char** instr_argv)
{
    errno = 0;
    char* source = realpath(instr_argv[1], NULL);
    if ((source == NULL) && (errno == ENOENT))
    {
        fprintf(stderr, ANSI_COLOR_RED "\nThe source directory doesn't exist!\n" ANSI_COLOR_RESET);
        free(source);
        return -1;
    }
    else if (source == NULL)
    {
        PERR("realpath");
        free(source);
        return -1;
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
                PERR("mkdir");
                for (int j = 0; j < (instr_argc - 2); j++)
                {
                    if (targets[j] != NULL)
                    {
                        free(targets[j]);
                    }
                }
                return -1;
            }
            targets[i] = realpath(instr_argv[i + 2], NULL);
        }
        else if (targets[i] == NULL)
        {
            PERR("realpath");
            for (int j = 0; j < (instr_argc - 2); j++)
            {
                if (targets[j] != NULL)
                {
                    free(targets[j]);
                }
            }
            return -1;
        }
    }

    free(instr_argv[1]);
    instr_argv[1] = source;
    for (int i = 2; i < instr_argc; i++)
    {
        free(instr_argv[i]);
        instr_argv[i] = targets[i - 2];
    }
    return 0;
}

void get_instruction(int instr_argc, char** instr_argv, struct BackupsMap* map, int ld)
{
    if (instr_argc > 2)
    {
        if (making_realpaths(instr_argc, instr_argv) == -1)
        {
            return;
        }
    }

    if ((strcmp(instr_argv[0], "add")) == 0)
    {
        if (instr_argc < 3)
        {
            usage(instr_argv[0], "<source path> <target path 1> <target path 2> ...");
            return;
        }

        char* source = instr_argv[1];
        size_t src_len = strlen(source);

        for (int i = 2; i < instr_argc; i++)
        {
            int correct = 0;

            char* target = instr_argv[i];
            // czy nie jest wewnatrz source
            if (strncmp(source, target, src_len) == 0)
            {
                if (target[src_len] == '/' || target[src_len] == '\0')
                {
                    fprintf(stderr, ANSI_COLOR_RED "\nTarget '%s' is inside source '%s'!\n" ANSI_COLOR_RESET, target,
                            source);
                    correct = -1;
                }
            }
            // czy jest pusty
            if (isEmpty(instr_argv[i]) != 0)
            {
                correct = -1;
            }

            // czy nie ma juz tam backupu
            for (int j = 0; j < map->backup_count; j++)
            {
                if (strcmp(map->backup_map[j].target, instr_argv[i]) == 0)
                {
                    fprintf(stderr, ANSI_COLOR_RED "\nTarget '%s' already contains a backup!\n" ANSI_COLOR_RESET,
                            instr_argv[i]);
                    correct = -1;
                    break;
                }
            }

            if (correct == 0)
            {
                int ret = fork();
                switch (ret)
                {
                    case (-1):
                        PERR("fork");
                        return;
                    case (0):
                        add_action(instr_argv[1], instr_argv[i], ld);
                        exit(EXIT_SUCCESS);
                        break;
                    default:
                        add_to_BackupMap(map, ret, instr_argv[1], instr_argv[i]);
                        break;
                }
            }
        }
    }
    else if ((strcmp(instr_argv[0], "end")) == 0)
    {
        if (instr_argc < 3)
        {
            usage(instr_argv[0], "<source path> <target path 1> <target path 2> ...");
            return;
        }

        for (int i = 2; i < instr_argc; i++)
        {
            end_action(map, instr_argv[1], instr_argv[i]);
        }
    }
    else if ((strcmp(instr_argv[0], "restore")) == 0)
    {
        if (instr_argc != 3)
        {
            usage(instr_argv[0], "<source path> <target path>");
            return;
        }

        restore_function(instr_argv[1], instr_argv[2], map);
    }
    else if ((strcmp(instr_argv[0], "exit") == 0))
    {
        if (instr_argc != 1)
        {
            usage(instr_argv[0], NULL);
            return;
        }

        exit_function(map);
    }
    else if ((strcmp(instr_argv[0], "list")) == 0)
    {
        if (instr_argc != 1)
        {
            usage(instr_argv[0], NULL);
            return;
        }

        list_action(map);
    }
    else
    {
        fprintf(stderr, ANSI_COLOR_RED "\nUnrecognizable instruction!\n" ANSI_COLOR_RESET);
        return;
    }
}

void detect_crashed_backups(struct BackupsMap* map)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        struct backup* b = NULL;
        for (int i = 0; i < map->backup_count; i++)
        {
            if (map->backup_map[i].pid == pid)
            {
                b = &map->backup_map[i];
                break;
            }
        }

        if (b != NULL)
        {
            fprintf(stderr, ANSI_COLOR_RED "\nBackup process %d failed! Removed from list.\n " ANSI_COLOR_RESET, pid);

            remove_from_BackupMap(map, pid);
            fflush(stderr);
        }
    }
    backup_crashed = 0;
    return;
}

void hello()
{
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
}

int main(int argc, char** argv)
{
    if (argc != 1)
    {
        usage(argv[0], NULL);
        exit(EXIT_FAILURE);
    }

    int ld;
    if ((ld = TEMP_FAILURE_RETRY(open("actions.log", O_WRONLY | O_CREAT | O_APPEND, 0777))) < 0)
    {
        ERR("open");
    }

    ignore_signals();
    sethandler(handle_exit_signal, SIGINT);
    sethandler(handle_exit_signal, SIGTERM);
    sethandler(handle_backup_crashed, SIGCHLD);

    hello();

    size_t len = 0;
    char* input = NULL;
    char* instr_argv[MAX_ARG] = {0};
    int instr_argc = 0;

    struct BackupsMap map = {0};

    while (app_running)
    {
        if (backup_crashed == 1)
        {
            detect_crashed_backups(&map);
            backup_crashed = 0;
        }
        printf(ANSI_BOLD "\nWrite a command and press ENTER\n> " ANSI_COLOR_RESET);
        errno = 0;
        if (getline(&input, &len, stdin) == -1)
        {
            if (errno == EINTR)
            {
                if (backup_crashed == 1)
                {
                    detect_crashed_backups(&map);
                }
                clearerr(stdin);
                continue;
            }
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
            get_instruction(instr_argc, instr_argv, &map, ld);
        }

        for (int i = 0; i < instr_argc; i++)
        {
            free(instr_argv[i]);
            instr_argv[i] = NULL;
        }
    }

    cleanup(input, instr_argv, instr_argc, &map);

    if (TEMP_FAILURE_RETRY(close(ld)))
    {
        ERR("close");
    }

    return 0;
}
