#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 200

int main()
{
    int batch_mode = 0;

    int exitFlag = 0;

    char cmdRaw[MAX];
    char *cmd;
    char * out;
    char curDirectory[50];

    while(exitFlag == 0){
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
                printf("\n%s\n\n", "Input error!");
                exit(1);
            }

            int incrementor = 0;

            while(cmdRaw[incrementor] != '\n' && cmdRaw[incrementor] != '\0'){


                incrementor++;
            }

            if(cmdRaw[incrementor] == '\n'){
                cmdRaw[incrementor] = '\0';
            }

            out = (char*) malloc(incrementor * sizeof(char));

            strcpy(out, cmdRaw);
            printf("ok");

        }

        else{
            exit(1);
        }

        char * token = strtok(out, " ");

        if(strcmp(token, "quit") == 0){ // broken. new string needs to be allocated and individual words need to be tokenizedt
            printf("\n\n%s\n", "Exiting...");
            exit(0);
        }
        else if(strcmp(token, "cd") == 0){
            printf("\n\n%s\n", "You selected cd");
        }

    }

    return 0;
}

