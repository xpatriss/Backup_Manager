#include "list_command.h"

void list_action(struct BackupsMap *map) {
  // sprawdź i posprzątaj backupy, których foldery docelowe zostały usunięte z
  // dysku
  for (int i = 0; i < map->backup_count; i++) {
    if (access(map->backup_map[i].target, F_OK) != 0) {
      printf(ANSI_COLOR_YELLOW
             "\nTarget directory '%s' was removed externally. Terminating "
             "backup job [pid = %d]...\n" ANSI_COLOR_RESET,
             map->backup_map[i].target, map->backup_map[i].pid);

      kill(map->backup_map[i].pid, SIGTERM);
      waitpid(map->backup_map[i].pid, NULL, 0);

      remove_from_BackupMap(map, map->backup_map[i].pid);
      i--; // cofamy indeks, bo remove_from_BackupMap podmienia element na
           // ostatni
    }
  }

  if (map->backup_count > 0) {
    printf(ANSI_BOLD "\nACTIVE BACKUPS:\n" ANSI_COLOR_RESET);
    for (int i = 0; i < map->backup_count; i++) {
      printf("%d. SOURCE DIRECTORY: '%s' BACKUP: '%s'\n", (i + 1),
             map->backup_map[i].source, map->backup_map[i].target);
    }
  } else {
    printf("\nThere is no active backups!\n");
  }
}