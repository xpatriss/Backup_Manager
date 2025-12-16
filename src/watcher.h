#ifndef WATCHER_H
#define WATCHER_H

#include "common.h"
#include "copy.h"
#include "delete.h"

struct Watch {
  int wd;     // key
  char *path; // value
};

struct WatchMap {
  struct Watch watch_map[MAX_WATCHES];
  int watch_count;
};

void update_watch_paths(struct WatchMap *map, const char *old_path,
                        const char *new_path);
void backuping(char *watched_dir, char *target, int ld);
void add_watch_recursive(int notify_fd, struct WatchMap *map,
                         char *watched_dir);
void remove_from_map(struct WatchMap *map, int wd);
struct Watch *find_watch(struct WatchMap *map, int wd);
void add_to_map(struct WatchMap *map, int wd, const char *path);
void clean_and_exit(struct WatchMap *map, int notify_fd, int exit_code);

#endif
