#include "end_command.h"

void end_action(struct BackupsMap* map, char* source, char* target)
{
    struct backup* backup_to_end = find_backup(map, target);
    if (backup_to_end == NULL)
    {
        fprintf(stderr, ANSI_COLOR_RED "\nThis backup doesn't exist!\n" ANSI_COLOR_RESET);
        return;
    }
    else
    {
        if (strcmp(source, backup_to_end->source) != 0)
        {
            fprintf(stderr, ANSI_COLOR_RED "\nThis backup doesn't exist!\n" ANSI_COLOR_RESET);
            return;
        }
        kill(backup_to_end->pid, SIGTERM);
        waitpid(backup_to_end->pid, NULL, 0);
        printf(ANSI_COLOR_YELLOW "\nBackup of '%s' in '%s' has ended.\n" ANSI_COLOR_RESET, source, target);
        remove_from_BackupMap(map, backup_to_end->pid);
        return;
    }
}
