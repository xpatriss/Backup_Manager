#include "delete.h"

void delete_file(char *source, char *target, char *filepath) {
  size_t src_len = strlen(source);
  char *newpath = filepath + src_len;
  if (*newpath == '/') {
    newpath++;
  }
  char *target_path = join_path(target, newpath);
  if ((remove(target_path)) != 0) {
    if (errno != ENOENT) {
      PERR("remove");
    }
    free(target_path);
    return;
  }
  free(target_path);
  return;
}

int recursive_clean(char *path) {
  DIR *d = opendir(path);
  if (!d) {
    PERR("opendir");
    return -1;
  }

  struct dirent *p;
  while ((p = readdir(d))) {
    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
      continue;

    char *full_path = join_path(path, p->d_name);

    struct stat st;

    if (lstat(full_path, &st) == 0) {
      if (S_ISDIR(st.st_mode)) {
        recursive_clean(full_path);
      } else {
        if (unlink(full_path) != 0) {
          PERR("unlink");
        }
      }
    }
    free(full_path);
  }

  closedir(d);

  return 0;
}

void delete_dir(char *source, char *target, char *dirpath) {
  size_t src_len = strlen(source);

  char *newpath = dirpath + src_len;
  if (*newpath == '/')
    newpath++;

  char *target_path = join_path(target, newpath);

  recursive_clean(target_path);

  if (rmdir(target_path) != 0) {
    PERR("rmdir");
    return;
  }

  free(target_path);
}
