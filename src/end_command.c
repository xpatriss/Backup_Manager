#include "end_command.h"

void end_action(struct BackupsMap *map, char *source, char *target) {
  struct backup *backup_to_end = find_backup(map, target);
  if (backup_to_end == NULL) {
    fprintf(stderr, ANSI_COLOR_RED
            "\nThis backup is not currently working!\n" ANSI_COLOR_RESET);
    return;
  } else {
    if (strcmp(source, backup_to_end->source) != 0) {
      fprintf(stderr,
              ANSI_COLOR_RED
              "\n'%s' is not a backup of '%s'\n" ANSI_COLOR_RESET,
              target, source);
      return;
    }
    remove_from_BackupMap(map, backup_to_end->pid);
    kill(backup_to_end->pid, SIGTERM);
    waitpid(backup_to_end->pid, NULL, 0);
    printf(ANSI_COLOR_YELLOW
           "\nBackup of '%s' in '%s' has ended.\n" ANSI_COLOR_RESET,
           source, target);
    return;
  }
}
