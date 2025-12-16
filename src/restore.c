#include "restore.h"

void restore_function(char *source, char *backup, struct BackupsMap *map) {
  struct backup *founded = find_backup(map, backup);
  if (founded != NULL) {
    fprintf(stderr, ANSI_COLOR_RED "\nThis backup is still in progress, you "
                                   "can't restore it!\n" ANSI_COLOR_RESET);
    return;
  } else {
    recursive_clean(source);
    file **content = scan_dir(backup);
    copy(backup, source, backup, source, content);
    free_file_pointer_tab(content);
    return;
  }
}
