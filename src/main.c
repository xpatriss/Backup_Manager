#include "common.h"
#include "parse.h"
#include "add_command.h"

int main(int argc, char** argv)
{
    if (argc != 1)
    {
        usage(argv[0], NULL);
        exit(EXIT_FAILURE);
    }

    /*printf(ANSI_BOLD "\nHello! Welcome to the Backup Manager. You can use one of the commands:\n" ANSI_COLOR_RESET);
    printf("\nadd <source path> <target path>  -  starts making a copy of <source path> in <target path>, you can add many target paths divided by spaces\n");
    printf("end <source path> <target paths>  -  breaks making a copy of <source path> in all <target paths>\n");
    printf("restore <source path> <target path>  -  restore a copy of <source path> from a <target path>\n");
    printf("list  -  shows a summary which contains folders which have a backup and their location\n");
    printf("exit  -  stops running the program\n");*/

    
    printf(ANSI_BOLD "\nHello! Welcome to the Backup Manager. You can use one of the commands:\n" ANSI_COLOR_RESET);
    printf("\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "add" ANSI_COLOR_RESET " <source> <target>       - starts making a copy of source in target (you can add many target paths divided by spaces)\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "end" ANSI_COLOR_RESET " <source> <target>       - breaks making a copy of source in target (you can add many target paths divided by spaces)\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "restore" ANSI_COLOR_RESET " <source> <target>   - restore a copy of source from target\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "list" ANSI_COLOR_RESET "                        - shows a summary of folders with backups\n");
    printf(ANSI_BOLD ANSI_COLOR_MAGENTA "exit" ANSI_COLOR_RESET "                        - stops running the program\n");
    printf("\n");
        
        size_t len = 0;
        char* input = NULL;

        while(1)
        {
            printf("\nI'm waiting for your command...\n");
            if (getline(&input, &len, stdin) == -1) {
                break; // Wyjście z pętli przy Ctrl+D
            }

            // Pomijamy puste linie (sam Enter)
            if (strcmp(input, "\n") == 0) {
                continue;
            }

            char* instr_argv[MAX_ARG] = {0};
            int instr_argc = parse(input, instr_argv);
            //printf("\nRESULT: argc = %d \ninstruction = %s \nsource = %s \ntarget = %s \n", instr_argc, instr_argv[0], instr_argv[1], instr_argv[2]);
            printf("\nDEBUG: argc = %d, instruction = %s\n", instr_argc, instr_argv[0]);
            if (instr_argc > 1) printf(" source = %s\n", instr_argv[1]);
            if (instr_argc > 2) printf(" target = %s\n", instr_argv[2]);

            for (int i = 0; i < instr_argc; i++)
            {
                free(instr_argv[i]);
            }
        }
    
    free(input);
    return 0;
}
