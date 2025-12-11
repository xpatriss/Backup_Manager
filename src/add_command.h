#ifndef ADD_COMMAND_H
#define ADD_COMMAND_H



void add_action(char* source, char* target);
void copy(char* source, char* target, elements content[3]);
elements* scan_dir(char* path, elements result[3]);
ssize_t bulk_write(int fd, char *buf, size_t count);
ssize_t bulk_read(int fd, char *buf, size_t count);

#endif