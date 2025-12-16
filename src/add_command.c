#include "add_command.h"

void add_action(char* source, char* target, int ld)
{
    
    file** content = scan_dir(source);

    copy(source, target, source, target, content);  // kopiuje elementy do katalogu docelowego

    backuping(source, target, ld);

    free_file_pointer_tab(content);

    return;
}
