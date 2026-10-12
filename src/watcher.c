#include "watcher.h"

void add_to_map(struct WatchMap *map, int wd, const char *path) {
  if (map->watch_count >= MAX_WATCHES) {
    fprintf(stderr, "Exceeded max watches!\n");
    return;
  }
  map->watch_map[map->watch_count].wd = wd;
  map->watch_map[map->watch_count].path = strdup(path);
  map->watch_count++;
}

struct Watch *find_watch(struct WatchMap *map, int wd) {
  for (int i = 0; i < map->watch_count; i++) {
    if (map->watch_map[i].wd == wd) {
      return &map->watch_map[i];
    }
  }
  return NULL;
}

void remove_from_map(struct WatchMap *map, int wd) {
  for (int i = 0; i < map->watch_count; i++) {
    if (map->watch_map[i].wd == wd) {
      free(map->watch_map[i].path);
      map->watch_map[i] = map->watch_map[map->watch_count - 1];
      map->watch_count--;
      return;
    }
  }
}

void add_watch_recursive(int notify_fd, struct WatchMap *map,
                         char *watched_dir) {
  uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO |
                  IN_CLOSE_WRITE | IN_ATTRIB;

  int wd = inotify_add_watch(notify_fd, watched_dir, mask);
  if (wd < 0) {
    ERR("inotify_add_watch");
    return;
  }
  add_to_map(map, wd, watched_dir);

  DIR *dir = opendir(watched_dir);
  if (!dir) {
    PERR("opendir");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char *full_path = join_path(watched_dir, entry->d_name);

    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
      add_watch_recursive(notify_fd, map, full_path);
    }

    free(full_path);
  }

  closedir(dir);
}

void update_watch_paths(struct WatchMap *map, const char *old_path,
                        const char *new_path) {
  size_t old_len = strlen(old_path);

  for (int i = 0; i < map->watch_count; i++) {
    if (strncmp(map->watch_map[i].path, old_path, old_len) == 0 &&
        (map->watch_map[i].path[old_len] == '/' ||
         map->watch_map[i].path[old_len] == '\0')) {
      char new_full_path[PATH_MAX];
      snprintf(new_full_path, sizeof(new_full_path), "%s%s", new_path,
               map->watch_map[i].path + old_len);

      free(map->watch_map[i].path);
      map->watch_map[i].path = strdup(new_full_path);
    }
  }
}

void clean_and_exit(struct WatchMap *map, int notify_fd, int exit_code) {
  if (map != NULL) {
    for (int i = 0; i < map->watch_count; i++) {
      if (map->watch_map[i].path != NULL) {
        free(map->watch_map[i].path);
      }
    }
    map->watch_count = 0;
  }

  if (notify_fd >= 0) {
    close(notify_fd);
  }

  exit(exit_code);
}

void backuping(char *watched_dir, char *target, int ld) {
  int notify_fd = inotify_init();
  if (notify_fd < 0) {
    ERR("inotify_init");
    return;
  }

  struct WatchMap watcher_map = {0};

  add_watch_recursive(notify_fd, &watcher_map, watched_dir);

  uint32_t pending_cookie = 0;
  char pending_move_path[PATH_MAX] = "";

  while (watcher_map.watch_count > 0) {
    char buffer[EVENT_BUF_LEN];
    errno = 0;
    ssize_t event_count = read(notify_fd, buffer, EVENT_BUF_LEN);
    if (event_count < 0) {
      if (errno == EINTR) {
        clean_and_exit(&watcher_map, notify_fd, EXIT_SUCCESS);
      }
      PERR("read");
      clean_and_exit(&watcher_map, notify_fd, EXIT_FAILURE);
    }

    ssize_t i = 0;

    while (i < event_count) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];

      struct Watch *watch = find_watch(&watcher_map, event->wd);

      char *event_path = NULL;
      if (watch && event->len > 0) {
        event_path = join_path(watch->path, event->name);
      } else if (watch) {
        event_path = strdup(watch->path);
      }

      if (event->mask & IN_IGNORED) {
        remove_from_map(&watcher_map, event->wd);
      } else {
        if (access(target, F_OK) != 0) {
          log_print(ld, getpid(), target,
                    "Target directory was removed. Exiting.", NULL);
          free(event_path);
          clean_and_exit(&watcher_map, notify_fd, EXIT_FAILURE);
        }
        if (event->mask & IN_ISDIR) {
          if (event->mask & IN_CREATE) {
            log_print(ld, getpid(), event_path, "was CREATED", NULL);
            add_watch_recursive(notify_fd, &watcher_map, event_path);
            file *element = make_file_struct(event->name, event_path);
            if (element == NULL) {
              free(event_path);
              i += sizeof(struct inotify_event) + event->len;
              continue;
            }
            size_t root_len = strlen(watched_dir);
            char *relative_dir = watch->path + root_len;
            char *full_target_path = join_path(target, relative_dir);
            copy_directory(watched_dir, target, full_target_path, element);
            free(full_target_path);
            free(element->name);
            free(element->real_path);
            free(element);
          }
          if (event->mask & IN_MOVED_TO) {
            log_print(ld, getpid(), event_path, "was MOVED TO", NULL);

            if (event->cookie == pending_cookie &&
                pending_cookie != 0) // ZMIENIONA NAZWA
            {
              update_watch_paths(&watcher_map, pending_move_path, event_path);

              file *element = make_file_struct(event->name, event_path);
              if (element == NULL) {
                free(event_path);
                i += sizeof(struct inotify_event) + event->len;
                continue;
              }
              size_t root_len = strlen(watched_dir);
              char *relative_dir = watch->path + root_len;
              char *full_target_path = join_path(target, relative_dir);
              copy_directory(watched_dir, target, full_target_path, element);
              free(full_target_path);
              free(element->name);
              free(element->real_path);
              free(element);

              pending_cookie = 0;
              pending_move_path[0] = '\0';
            } else // NOWY KATALOG
            {
              add_watch_recursive(notify_fd, &watcher_map, event_path);
              file *element = make_file_struct(event->name, event_path);
              if (element == NULL) {
                free(event_path);
                i += sizeof(struct inotify_event) + event->len;
                continue;
              }
              size_t root_len = strlen(watched_dir);
              char *relative_dir = watch->path + root_len;
              char *full_target_path = join_path(target, relative_dir);
              copy_directory(watched_dir, target, full_target_path, element);
              free(full_target_path);
              free(element->name);
              free(element->real_path);
              free(element);
            }
          }
          if (event->mask & IN_MOVED_FROM) {
            log_print(ld, getpid(), event_path, "was MOVED FROM", NULL);
            pending_cookie = event->cookie;
            strncpy(pending_move_path, event_path, sizeof(pending_move_path));

            delete_dir(watched_dir, target, event_path);
          }
          if (event->mask & IN_DELETE) {
            log_print(ld, getpid(), event_path, "was DELETED", NULL);
            delete_dir(watched_dir, target, event_path);
          }
          if (event->mask & IN_ATTRIB) {
            log_print(ld, getpid(), event_path, "CHANGED ATTRIBUTES", NULL);
            file *element = make_file_struct(event->name, event_path);
            if (element == NULL) {
              free(event_path);
              i += sizeof(struct inotify_event) + event->len;
              continue;
            }

            size_t root_len = strlen(watched_dir);
            char *relative_dir = watch->path + root_len;
            char *full_target_path = join_path(target, relative_dir);

            if (element->type == DIRECTORY_TYPE) {
              if (chmod(full_target_path, element->perms & 0777) == -1) {
                PERR("chmod");
              }
            }
            free(full_target_path);
            free(element->name);
            free(element->real_path);
            free(element);
          }
        } else if (event->mask & IN_CREATE && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "was CREATED", NULL);
          file *element = make_file_struct(event->name, event_path);
          if (element == NULL) {
            free(event_path);
            i += sizeof(struct inotify_event) + event->len;
            continue;
          }
          size_t root_len = strlen(watched_dir);
          char *relative_dir = watch->path + root_len;
          char *full_target_path = join_path(target, relative_dir);

          if (element->type == FILE_TYPE) {
            copy_file(full_target_path, element);
          } else if (element->type == SYMLINK_TYPE) {
            copy_symlink(watched_dir, target, watched_dir, full_target_path,
                         element);
          }
          free(full_target_path);
          free(element->name);
          free(element->real_path);
          free(element);
        } else if (event->mask & IN_MOVED_TO && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "was MOVED TO", NULL);
          file *element = make_file_struct(event->name, event_path);
          if (element == NULL) {
            free(event_path);
            i += sizeof(struct inotify_event) + event->len;
            continue;
          }
          size_t root_len = strlen(watched_dir);
          char *relative_dir = watch->path + root_len;
          char *full_target_path = join_path(target, relative_dir);

          if (element->type == FILE_TYPE) {
            copy_file(full_target_path, element);
          } else if (element->type == SYMLINK_TYPE) {
            copy_symlink(watched_dir, target, watched_dir, full_target_path,
                         element);
          }
          free(full_target_path);
          free(element->name);
          free(element->real_path);
          free(element);
        } else if (event->mask & IN_MOVED_FROM && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "was MOVED FROM", NULL);
          delete_file(watched_dir, target, event_path);
        } else if (event->mask & IN_CLOSE_WRITE && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "was MODIFIED", NULL);
          delete_file(watched_dir, target, event_path);
          file *element = make_file_struct(event->name, event_path);
          if (element == NULL) {
            free(event_path);
            i += sizeof(struct inotify_event) + event->len;
            continue;
          }
          size_t root_len = strlen(watched_dir);
          char *relative_dir = watch->path + root_len;
          char *full_target_path = join_path(target, relative_dir);

          if (element->type == FILE_TYPE) {
            copy_file(full_target_path, element);
          } else if (element->type == SYMLINK_TYPE) {
            copy_symlink(watched_dir, target, watched_dir, full_target_path,
                         element);
          }
          free(full_target_path);
          free(element->name);
          free(element->real_path);
          free(element);
        } else if (event->mask & IN_DELETE && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "was DELETED", NULL);
          delete_file(watched_dir, target, event_path);
        } else if (event->mask & IN_ATTRIB && !(event->mask & IN_ISDIR)) {
          log_print(ld, getpid(), event_path, "CHANGED ATTRIBUTES", NULL);
          file *element = make_file_struct(event->name, event_path);
          if (element == NULL) {
            free(event_path);
            i += sizeof(struct inotify_event) + event->len;
            continue;
          }

          size_t root_len = strlen(watched_dir);
          char *relative_dir = watch->path + root_len;
          char *full_target_path = join_path(target, relative_dir);

          if (element->type == FILE_TYPE) {
            if (chmod(full_target_path, element->perms & 0777) == -1) {
              PERR("chmod");
            }
          }
          free(full_target_path);
          free(element->name);
          free(element->real_path);
          free(element);
        }
      }

      i += sizeof(struct inotify_event) + event->len;
      free(event_path);
    }
  }

  clean_and_exit(&watcher_map, notify_fd, EXIT_SUCCESS);
}
