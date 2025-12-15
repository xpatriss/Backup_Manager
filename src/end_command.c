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
        printf("ending backup of %s: '%s' pid=%d\n", source, target, backup_to_end->pid);
        remove_from_BackupMap(map, backup_to_end->pid);
        return;
    }
}
