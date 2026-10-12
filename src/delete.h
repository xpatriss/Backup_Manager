#ifndef DELETE_H
#define DELETE_H

#include "common.h"

void delete_file(char *source, char *target, char *filepath);
void delete_dir(char *source, char *target, char *dirpath);
int recursive_clean(char *path);

#endif
