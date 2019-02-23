#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

            for(int j = 0; j < sizeof(cmdRaw); j++){
                cmdRaw[j] = '\0';
            }

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
            token = strtok(NULL, " "); // next input

            if(chdir(token) == 0){
                printf("\n\n%s\n", "Directory changed!");
            }
            else{
                printf("\n\n%s\n", "Error");
            }
        }
        else if(strcmp(token, "clr") == 0 || strcmp(token, "clear") == 0){
            printf("\e[1;1H\e[2J");
        }

        else{
            int pid = fork();

            if(pid == 0){ // child process

                char *const parmList[] = {token, "-a", NULL};
                int yo = execvp(parmList[0], parmList);

                if(yo != 0){
                    printf("\n\n%s\n", "-1 error");
                }

                printf("\n\n%s\n", "error with execution");
            }

            else{
                wait(NULL);
                printf("\n\n%s\n", "Back to parent process");
            }
        }

    free(out);

    }

    return 0;
}

