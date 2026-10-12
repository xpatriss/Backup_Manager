#ifndef RESTORE_H
#define RESTORE_H

#include "common.h"
#include "copy.h"
#include "delete.h"

void restore_function(char *source, char *backup, struct BackupsMap *map);

#endif
