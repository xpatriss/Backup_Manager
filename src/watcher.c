#include "watcher.h"

void add_to_map(struct WatchMap *map, int wd, const char *path) 
{
    if (map->watch_count >= MAX_WATCHES) {
        fprintf(stderr, "Exceeded max watches!\n");
        return;
    }
    map->watch_map[map->watch_count].wd = wd;
    map->watch_map[map->watch_count].path = strdup(path); // Must copy the path!
    map->watch_count++;
    //printf("new watch: '%s' @wd=%d\n", path, wd);
}

struct Watch *find_watch(struct WatchMap *map, int wd) 
{
    for (int i = 0; i < map->watch_count; i++) {
        if (map->watch_map[i].wd == wd) {
            return &map->watch_map[i];
        }
    }
    return NULL;
}

void remove_from_map(struct WatchMap *map, int wd) 
{
    for (int i = 0; i < map->watch_count; i++) {
        if (map->watch_map[i].wd == wd) {
            //printf("removing watch: '%s' @wd=%d\n", map->watch_map[i].path, wd);
            free(map->watch_map[i].path);
            map->watch_map[i] = map->watch_map[map->watch_count - 1];
            map->watch_count--;
            return;
        }
    }
}

void add_watch_recursive(int notify_fd, struct WatchMap *map, char *watched_dir) 
{
    uint32_t mask = IN_CREATE |      // Utworzono nowy plik/katalog
                IN_DELETE |      // Usunięto plik/katalog
                IN_MOVED_FROM |  // Zmieniono nazwę (stara nazwa)
                IN_MOVED_TO |    // Zmieniono nazwę (nowa nazwa)
                IN_CLOSE_WRITE | // Zakończono edycję pliku (zamiast IN_MODIFY)
                IN_ATTRIB;       // Zmieniono uprawnienia (chmod)

    int wd = inotify_add_watch(notify_fd, watched_dir, mask);
    if (wd < 0) 
    {
        perror("inotify_add_watch");
        return;
    }
    add_to_map(map, wd, watched_dir);

    DIR *dir = opendir(watched_dir);
    if (!dir) 
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char* full_path = join_path(watched_dir, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            add_watch_recursive(notify_fd, map, full_path);
        }

        free(full_path);
    }

    closedir(dir);
}

void backup(char* watched_dir, char* target)
{
    int notify_fd = inotify_init();
    if (notify_fd < 0) {
        perror("inotify_init");
        return;
    }

    struct WatchMap map = {0};

    add_watch_recursive(notify_fd, &map, watched_dir);

    while (1) 
    {
        char buffer[EVENT_BUF_LEN];
        ssize_t event_count = read(notify_fd, buffer, EVENT_BUF_LEN);

        if (event_count < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        ssize_t i = 0;
        while (i < event_count) 
        {
            struct inotify_event* event = (struct inotify_event*) &buffer[i];

            struct Watch *watch = find_watch(&map, event->wd);

            char* event_path = NULL;
            //char event_path[PATH_MAX] = "";
            if (watch && event->len > 0) 
            {
                event_path = join_path(watch->path, event->name);
            } 
            else if (watch) 
            {
                // Event has no name, use the watch path (e.g., IN_DELETE_SELF)
                // strncpy(event_path, watch->path, sizeof(event_path));
                event_path = strdup(watch->path);
            }

            if (event->mask & IN_IGNORED) 
            {
                // Watch was removed by the kernel
                remove_from_map(&map, event->wd);
            }
            else 
            {

                if (event->mask & IN_ISDIR) 
                {
                    
                    continue;
                    // Handle tree insertions
                    if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) 
                    {
                        add_watch_recursive(notify_fd, &map, event_path);
                        //add dir//
                    }
                    if (event->mask & IN_MOVED_FROM)
                    {
                        //delete dir//
                    }
                } 
                else if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
                {
                    printf("\n'%s' was CREATED / MOVED TO", event_path);
                    fflush(stdout);
                    file* element = make_file_struct(event->name, event_path);
                    if (element->type == FILE_TYPE) 
                    { 
                        copy_file(target, element);                    
                    }
                    else if (element->type == SYMLINK_TYPE)
                    {
                        copy_symlink(watched_dir, target, watched_dir, target, element);
                    }
                    free(element->name);
                    free(element->real_path);
                    free(element);
                }
                else if (event->mask & IN_MOVED_FROM)
                {
                    printf("\n'%s' was MOVED FROM", event_path);
                    fflush(stdout);
                    delete_file(watched_dir, target, event_path);
                }
                else if (event->mask & IN_CLOSE_WRITE)
                {
                    printf("\n'%s' was MODIFIED", event_path);
                    fflush(stdout);
                    delete_file(watched_dir, target, event_path);
                    file* element = make_file_struct(event->name, event_path);
                    copy_file(target, element);
                    free(element->name);
                    free(element->real_path);
                    free(element);
                }
                else if (event->mask & IN_DELETE)
                {
                    printf("\n'%s' was DELETED", event_path);
                    fflush(stdout);
                    delete_file(watched_dir, target, event_path);
                }
            } 
            

            i += sizeof(struct inotify_event) + event->len;
            free(event_path);
        }
    }

    close(notify_fd);
    return;
}