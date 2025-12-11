#include "common.h"
#include "parse.h"
#include "add_command.h"

int main(int argc, char** argv)
{
    if (argc != 1)
    {
        usage(argv[0], NULL);
    }

    printf("\nHello! Welcome to the Backup Manager. You can use one of the commands:\n");
    printf("\nadd <source path> <target path>  -  starts making a copy of <source path> in <target path>, you can add many target paths divided by spaces\n");
    printf("end <source path> <target paths>  -  breaks making a copy of <source path> in all <target paths>\n");
    printf("restore <source path> <target path>  -  restore a copy of <source path> from a <target path>\n");
    printf("list  -  shows a summary which coontains folders which have a backup and their location\n");
    printf("exit  -  stops running the program\n");

    printf("\nI'm waiting for your command...\n");


    char* input = NULL;
    size_t len;

    getline(&input, &len, stdin);
    
    char* instr_argv[MAX_ARG];

    int instr_argc = parse(input, instr_argv);

    printf("\nRESULT: argc = %d \ninstruction = %s \nsource = %s \ntarget = %s \n", instr_argc, instr_argv[0], instr_argv[1], instr_argv[2]);

    free(input);
    for (int i = 0; i < instr_argc; i++)
    {
        free(instr_argv[i]);
    }

    return 0;
}
