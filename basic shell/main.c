#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int batch_mode = 0;

    char cmdRaw[4];
    char *cmd;
    char curDirectory[50];

    if(batch_mode == 0){
        printf("%s", "myshell");

        if(getcwd(curDirectory, sizeof(curDirectory)) != NULL){
            printf("%s%s%s", ": ", curDirectory, "> ");
        }
        else{
            printf("%s", "> ");
        }

        if(fgets(cmdRaw, sizeof(cmdRaw), stdin)){

        }
        else{
            printf("\n&s\n\n", "Input error!");
            exit(1);
        }
    }

    else{
        exit(1);
    }

    if(strcmp(cmdRaw, "quit") == 0){ // broken. new string needs to be allocated and individual words need to be tokenized
        printf("\n\n%s\n", "Exiting...");
    }
    return 0;
}
