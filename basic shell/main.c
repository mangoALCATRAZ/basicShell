#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX 200

int prepareOutputRedirection(char **token, char **filename);
int prepareInputRedirection(char **token, char **filename);
int forkLogic(char* args[], int inRedirect, int outRedirect);
int pipeLogic(char* args[], char* pipeArgs[]);



int main()
{
    int batch_mode = 0;

    int exitFlag = 0;

    int inRedirect;
    int outRedirect;
    char * inRedirectFilename;
    char * outRedirectFilename;


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
            int inputRedir = 0;
            int outputTruncate = 0;
            int outputAppend = 0;
            int errorFlag = 0;

            *next++ = token;
            token = strtok(NULL, " ");

            while(token != NULL){
                if(strcmp(token, "|") == 0){
                    pipeFlag = 1;
                    break;
                }
                else if(strcmp(token, "<") == 0){
                    inputRedir = 1;
                    break;
                }
                else if(strcmp(token, ">") == 0){
                    outputTruncate = 1;
                    break;
                }
                else if(strcmp(token, ">>") == 0){
                    outputAppend = 1;
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

            else if(inputRedir == 1 && errorFlag == 0){
                char ** tokenP = &token;
                char ** inP = &inRedirectFilename;

                int prepareInputResult = prepareInputRedirection(tokenP, inP);
                if(prepareInputResult < 0){
                    errorFlag = 1;
                }
                else if(prepareInputResult == 1){
                    outputTruncate = 1;
                }
                else if(prepareInputResult == 2){
                    outputAppend = 1;
                }
            }
            else if(outputTruncate == 1 && errorFlag == 0){
                char ** tokP = &token;
                char ** outP = &outRedirectFilename;
                int prepareOutputTruncResult = prepareOutputRedirection(tokP, outP);
                if(prepareOutputTruncResult < 0){
                    errorFlag = 1; // output redirection filename obtaining failed
                }
                else if(prepareOutputTruncResult > 0){
                    if(inputRedir == 1){ // cannot have multiple input redirections
                        errorFlag = 1;
                    }
                    else{
                        char ** inP2 = &inRedirectFilename;
                        int inAfterOutResult = prepareInputRedirection(tokP, inP2);
                        if(inAfterOutResult < 0){ // input redirection filename obtaining failed
                            errorFlag = 1;
                        }
                        else if(inAfterOutResult > 0){ // cannot have multiple input or output redirections
                            errorFlag = 1;
                        }
                        else{
                            inputRedir = 1;
                        }
                    }
                }
            }
            else if(outputAppend == 1 && errorFlag == 0){
                char ** tokP = &token;
                char ** outP = &outRedirectFilename;
                int prepareOutputAppendResult = prepareOutputRedirection(tokP, outP);
                if(prepareOutputAppendResult < 0){
                    errorFlag = 1;
                }
                else if(prepareOutputAppendResult > 0){
                    if(inputRedir == 1){
                        errorFlag = 1; // cannot have multiple input redirections
                    }
                    else{
                        char ** inP2 = &inRedirectFilename;
                        int inAfterAppendOutResult = prepareInputRedirection(tokP, inP2);
                        if(inAfterAppendOutResult < 0){
                            errorFlag = 1;
                        }
                        else if(inAfterAppendOutResult > 0){
                            errorFlag = 1;
                        }
                        else{
                            inputRedir = 1;
                        }
                    }
                }
            }

            if(errorFlag == 0){ // if no errors, proceed
                if(pipeFlag == 1){
                    pipeLogic(args, pipeArgs);
                }
                else{
                    if(inputRedir == 1){
                        inRedirect = open(inRedirectFilename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);
                        if(inRedirect == -1){
                            errorFlag = 1;
                        }
                    }
                    if(outputAppend == 1 && errorFlag == 0){
                        outRedirect = open(outRedirectFilename, O_RDWR | O_CREAT | O_APPEND, S_IRWXU | S_IRWXO);
                        if(outRedirect == -1){
                            errorFlag = 2;
                        }
                    }
                    else if(outputTruncate == 1 && errorFlag == 0){
                        outRedirect = open(outRedirectFilename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO);
                        if(outRedirect == -1){
                            errorFlag = 2;
                        }
                    }

                    if(errorFlag == 0){
                        if(inputRedir == 1 && outputTruncate == 0 && outputAppend == 0){
                            forkLogic(args, inRedirect, -1);
                        }
                        else if(inputRedir == 1 && (outputTruncate == 1 || outputAppend == 1)){
                            forkLogic(args, inRedirect, outRedirect);
                        }
                        else if(inputRedir == 0 && outputTruncate == 0 && outputAppend == 0){
                            forkLogic(args, -1, -1);
                        }
                        else{
                            forkLogic(args, -1, outRedirect);
                        }
                    }
                    else{
                        if(errorFlag == 1){
                            printf("\n\n%s\n", "Error in opening or creating input redirection file for this command.");
                        }
                        else if(errorFlag == 2){
                            printf("\n\n%s\n", "Error in opening or creating output redirection file for this command");
                        }
                    }
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

int prepareOutputRedirection(char **token, char **filename){
    int ret = 0;

    *token = strtok(NULL, " ");

    if(*token == NULL){ // may need to add conditions for additional special character input in lieu of acceptable filename. watch out for this in testing
        ret = -1; //error
    }
    else{
        *filename = *token;
        *token = strtok(NULL, " ");
        if(*token != NULL){
            if(strcmp(*token, "<") != 0){
                ret = -1; //error
            }
            else if(strcmp(*token, "<") == 0){
                ret = 1; // additional input redirection detected
            }
        }
    }

    return ret;
}

int prepareInputRedirection(char **token, char **filename){
    int ret = 0;

    *token = strtok(NULL, " ");

    if(*token == NULL){ // see equivalent comment in prepareOutputRedirection()
        ret = -1; // error
    }
    else{
        *filename = *token;
        *token = strtok(NULL, " ");
        if(*token != NULL){
            if(strcmp(*token, ">") != 0 && strcmp(*token, ">>") != 0){
                ret = -1; // error
            }
            else if(strcmp(*token, ">") == 0){
                ret = 1; // additional output redirection detected.
            }
            else if(strcmp(*token, ">>") == 0){
                ret = 2; // additional output append redirection detected
            }
        }
    }
    return ret;
}

int forkLogic(char* args[], int inRedirect, int outRedirect){
    int pid;
    int ret = 0;

    int inFlag = 0;
    int outFlag = 0;

    pid = fork();

    if(pid < 0){
        printf("\n\nForking error!");
        ret = 1;
    }
    else if(pid == 0){ // CHILD PROCESS
        if(inRedirect != -1){
            int check1 = dup2(inRedirect, 0);
            inFlag = 1;
        }
        if(outRedirect != -1){
            int check2 = dup2(outRedirect, 1);
            outFlag = 1;
        }

        int execute = execvp(args[0], args);
        if(execute != 0){
            printf("\n\nExecution Error in fork");
            exit(0);
        }
    }
    else{ // PARENT PROCESS
        wait(NULL);
        if(inFlag == 1){
            close(inRedirect);
        }
        if(outFlag == 1){
            close(outRedirect);
        }
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

