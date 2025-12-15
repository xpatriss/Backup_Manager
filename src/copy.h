#ifndef COPY_H
#define COPY_H

#include "common.h"

void copy(char* s_root, char* t_root, char* source, char* target, file** content);
void copy_symlink(char* s_root, char* t_root, char* source, char* target, file* s);
void copy_directory(char* s_root, char* t_root, char* target, file* d);
void copy_file(char* target, file* f);

#endif
