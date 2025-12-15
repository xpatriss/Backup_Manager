#include "list_command.h"

void list_action(struct BackupsMap* map)
{
    if (map->backup_count > 0)
    {
        for (int i = 0; i < map->backup_count; i++)
        {
            printf("%d. directory: '%s' backup: '%s'\n", (i + 1), map->backup_map[i].source, map->backup_map[i].target);
        }
    }
    else
    {
        printf("There is no active backups!\n");
        return;
    }
    return;
}
