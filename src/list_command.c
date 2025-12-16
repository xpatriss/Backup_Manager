#include "list_command.h"

void list_action(struct BackupsMap *map) {
  if (map->backup_count > 0) {
    printf(ANSI_BOLD "\nACTIVE BACKUPS:\n" ANSI_COLOR_RESET);
    for (int i = 0; i < map->backup_count; i++) {
      printf("%d. SOURCE DIRECTORY: '%s' BACKUP: '%s'\n", (i + 1),
             map->backup_map[i].source, map->backup_map[i].target);
    }
  } else {
    printf("\nThere is no active backups!\n");
  }
  return;
}
