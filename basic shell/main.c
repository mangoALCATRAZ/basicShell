#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 200

int forkLogic(char* args[]);
int pipeLogic(char* args[], char* pipeArgs[]);


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

        if(token == NULL){
            // nothing
        }
        else if(strcmp(token, "quit") == 0){ // broken. new string needs to be allocated and individual words need to be tokenizedt
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
            char *args[64];
            char **next = args;

            char *pipeArgs[64];
            char **nextPipe = pipeArgs;

            int pipeFlag = 0;
            int errorFlag = 0;

            *next++ = token;
            token = strtok(NULL, " ");

            while(token != NULL){
                if(strcmp(token, "|") == 0){
                    pipeFlag = 1;
                    break;
                }
                *next++ = token;
                token = strtok(NULL, " ");
            }


            *next = NULL;

            if(pipeFlag == 1){
                token = strtok(NULL, " ");

                if(token == NULL){
                    errorFlag = 1;
                }
                else{
                    while(token != NULL){
                        *nextPipe++ = token;
                        token = strtok(NULL, " ");
                    }

                    *nextPipe = NULL;
                }
            }

            if(errorFlag == 0){ // if no errors, proceed
                if(pipeFlag == 1){
                    pipeLogic(args, pipeArgs);
                }
                else{
                    forkLogic(args);
                }
            }
            else{
                printf("\n\n%s\n", "Error: invalid input");
            }
        }

    free(out);

    }

    return 0;
}

int forkLogic(char* args[]){
    int pid;
    int ret = 0;

    pid = fork();

    if(pid < 0){
        printf("\n\nForking error!");
        ret = 1;
    }
    else if(pid == 0){ // CHILD PROCESS
        int execute = execvp(args[0], args);
        if(execute != 0){
            printf("\n\nExecution Error in fork");
            exit(0);
        }
    }
    else{ // PARENT PROCESS
        wait(NULL);
    }

    return ret;
}

int pipeLogic(char* args[], char* pipeArgs[]){
    int pid;
    int pid2;
    int fd[2];
    int ret = 0;

    if(pipe(fd) < 0){
        printf("Error in piping");
        ret = 1;
    }
    else{
        pid = fork();
        if(pid < 0){
            printf("Forking error!");
            ret = 1;
        }

        else if(pid == 0){ // CHILD
            close(fd[0]);
            dup2(fd[1], 1);
            close(fd[1]);

            int execute = execvp(args[0], args);
            if(execute != 0){
                printf("\n\n%s\n", "-1 error");
                exit(0);
            }
        }
        else{ // PARENT
            pid2 = fork();

            if(pid2 < 0){
                printf("fork error");
                ret = 1;
            }
            else if(pid2 == 0){ // NEW CHILD
                close(fd[1]);
                dup2(fd[0], 0);
                close(fd[0]);
                int execute2 = execvp(pipeArgs[0], pipeArgs);
                if(execute2 < 0){
                    printf("-1 error");
                    exit(0);
                }
            }
            else{ // NEW PARENT
                close(fd[0]);
                close(fd[1]);
                waitpid(pid, NULL, WCONTINUED);
                waitpid(pid2, NULL, WCONTINUED);
            }
        }

    }
    return ret;
}

