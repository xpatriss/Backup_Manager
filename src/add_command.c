#include "add_command.h"

void add_action(char* source, char* target)
{
    errno = 0;
    DIR* dirp;
    if ((dirp = opendir(target)) == NULL)
    {
        /*if (errno == ENOENT)
        {
            if (mkdir(target, 0777))
            {
                perror("mkdir failed");
                return;
            }
        }
        else*/
        //{
            perror("opendir failed");
            return;
        //}
    }
    else
    {
        

        if (closedir(dirp))
        {
            perror("closedir failed");
            return;
        }
    }

    file** content = scan_dir(source);

    copy(source, target, source, target, content);  // kopiuje elementy do katalogu docelowego

    backuping(source, target);

    free_file_pointer_tab(content);
}
