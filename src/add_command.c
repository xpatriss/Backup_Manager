#include "add_command.h"

void add_action(char* source, char* target)
{
    errno = 0;
    DIR* dirp;
    if ((dirp = opendir(target)) == NULL)
    {
        if (errno == ENOENT)
        {
            if (mkdir(target, 0777))
            {
                perror("mkdir failed");
                return;
            }
        }
        else
        {
            perror("opendir failed");
            return;
        }
    }
    else
    {
        file** target_content = scan_dir(target);
        if (target_content[0] != NULL)
        {
            fprintf(stderr, ANSI_COLOR_RED "\nTarget directory in not empty!\n" ANSI_COLOR_RESET);
            free_file_pointer_tab(target_content);
            if (closedir(dirp))
            {
                perror("closedir failed");
                return;
            }
            return;
        }

        free_file_pointer_tab(target_content);

        if (closedir(dirp))
        {
            perror("closedir failed");
            return;
        }
    }

    file** content = scan_dir(source);

    copy(source, target, source, target, content);  // kopiuje elementy do katalogu docelowego

    backup(source, target);

    free_file_pointer_tab(content);
}
