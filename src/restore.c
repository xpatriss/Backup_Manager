#include "restore.h"

void restore_function(char *source, char *backup, struct BackupsMap *map) {
  struct backup *founded = find_backup(map, backup);
  if (founded != NULL) {
    fprintf(stderr, ANSI_COLOR_RED "\nThis backup is still in progress, you "
                                   "can't restore it!\n" ANSI_COLOR_RESET);
    return;
  }

  // sprawdź, czy katalog backupu istnieje i jest katalogiem
  struct stat st_backup;
  if (stat(backup, &st_backup) != 0 || !S_ISDIR(st_backup.st_mode)) {
    fprintf(stderr,
            ANSI_COLOR_RED "\nError: Backup directory '%s' does not exist or "
                           "is not a directory!\n" ANSI_COLOR_RESET,
            backup);
    return;
  }

  // odczytaj zawartość backupu
  file **content = scan_dir(backup);
  if (content == NULL) {
    fprintf(stderr,
            ANSI_COLOR_RED "\nError: Failed to read backup directory '%s'. "
                           "Restore aborted.\n" ANSI_COLOR_RESET,
            backup);
    return;
  }

  // zabezpieczenie: jeśli backup jest pusty, przerwij operację, aby nie
  // wyczyścić źródła do zera
  if (content[0] == NULL) {
    fprintf(stderr,
            ANSI_COLOR_RED "\nError: Backup directory '%s' is empty! Restore "
                           "aborted to prevent data loss.\n" ANSI_COLOR_RESET,
            backup);
    free_file_pointer_tab(content);
    return;
  }

  // upewnij się, że katalog źródłowy istnieje (jeśli nie, utwórz go)
  struct stat st_source;
  if (stat(source, &st_source) != 0) {
    if (mkdir(source, 0700) != 0) {
      PERR("mkdir source");
      free_file_pointer_tab(content);
      return;
    }
  } else if (!S_ISDIR(st_source.st_mode)) {
    fprintf(stderr,
            ANSI_COLOR_RED "\nError: Source path '%s' exists but is not a "
                           "directory!\n" ANSI_COLOR_RESET,
            source);
    free_file_pointer_tab(content);
    return;
  }

  // bezpieczne czyszczenie źródła i przywrócenie danych
  recursive_clean(source);
  copy(backup, source, backup, source, content);
  free_file_pointer_tab(content);

  printf(
      ANSI_COLOR_GREEN
      "\nRestore from '%s' to '%s' completed successfully.\n" ANSI_COLOR_RESET,
      backup, source);
}