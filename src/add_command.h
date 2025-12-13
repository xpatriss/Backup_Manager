#ifndef ADD_COMMAND_H
#define ADD_COMMAND_H



void add_action(char* source, char* target);
void copy(char* s_root, char* t_root, char* source, char* target, file** content);
void copy_symlink(char* s_root, char* t_root, char* source, char* target, file* s);
file** scan_dir(char* path);
void copy_file(char* target, file* f);
ssize_t bulk_write(int fd, char *buf, size_t count);
ssize_t bulk_read(int fd, char *buf, size_t count);

#endif