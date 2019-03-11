#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX 200

int prepareOutputRedirection(char **token, char **filename);
int builtInOutputRedirect(char ** token, char ** filename, int * file, int * saved_stdout);
int prepareInputRedirection(char **token, char **filename);
int forkLogic(char* args[], int inRedirect, int outRedirect);
int pipeLogic(char* args[], char* pipeArgs[]);



int main(int argc, char *argv[])
{
    int batch_mode = 0; // flag for batch mode
    FILE *batchIn; // File pointer for batch file

    int exitFlag = 0; // flag that signals when the

    int inRedirect; // file pointer for input redirection source
    int outRedirect; // file pointer for output redirection source
    char * inRedirectFilename; // filename for input redirection
    char * outRedirectFilename; // filename for output redirection

    int increm = 0;

    char cmdRaw[MAX]; // raw, untrimmed command input with extra whitespace
    char *cmd;
    char * out; // trimmed, final command with no trailing whitespace and null-terminated
    char curDirectory[50]; // current directory in shell


    // PROGRAM BEGIN //

    if (argc >= 2){ // if there are arguments in the command line
        batch_mode = 1; // batch mode is activated for this instance of the shell
        char const* const fileName = argv[1]; // this is the argument given, this shell only takes one, which is the filename for the batch file
        batchIn = fopen(fileName, "r"); // FILE open
        if(batchIn == NULL){ // error handling for if file cannot be found
            printf("\n\nError opening Batch File. Proceeeding with normal terminal function.");
            batch_mode = 0;
        }
    }

    // BELOW IS THE MAIN LOOP OF THE PROGRAM. tHE PROGRAM CONTINUES UNTIL
    // 1. exit() is called
    // or
    // 2. exitFlag is triggered to 1 and program finishes naturally

    while(exitFlag == 0){
        char * get = getcwd(curDirectory, sizeof(curDirectory));

        // first time the loop executes, calling directory is appended to PATH environment variable for execvp to use.
        if(increm == 0){
            char * getpath = getenv("PATH");
            strcat(getpath, ";");
            strcat(getpath, curDirectory);

            if(setenv("PATH", getpath, 0) != 0){
                printf("\n\nerror\n");
            }
        }
        increm++;


        // batch_mode = 0 is normal user command line input, where prompts are given and commands are taken from user.
        // commands in batch_mode and non-batch-mode are the same. the input is just different
        if(batch_mode == 0){ //
            printf("%s", "myshell"); // myshell prompt

            for(int j = 0; j < sizeof(cmdRaw); j++){ // initialize entire cmdRAW string to NULL to protect it from leftover data from looping
                cmdRaw[j] = '\0';
            }

            if(get != NULL){ //check for valid current directory
                printf("%s%s%s", ": ", curDirectory, "> "); // if valid print directory and prompt
            }
            else{
                printf("%s", "> "); // otherwise just print prompt
            }

            if(fgets(cmdRaw, sizeof(cmdRaw), stdin)){ // wait for user input and save to cmdRAW, which is purposefully too big to accommodate it

            }
            else{ // error handling
                printf("\n%s\n\n", "Input error!");
                exit(1);
            }

            int incrementor = 0;

            while(cmdRaw[incrementor] != '\n' && cmdRaw[incrementor] != '\0'){ // counts number of character in input to make malloc'd trimmed string


                incrementor++;
            }

            if(cmdRaw[incrementor] == '\n'){ // replaces newline character typed by user in input with null terminator
                cmdRaw[incrementor] = '\0';
            }

            out = (char*) malloc(incrementor * sizeof(char)); // creates malloc'd trimmed string, this is what will be parsed through

            strcpy(out, cmdRaw); // copies contents of raw string to trimmed string

        }


        else if(batch_mode == 1){ // batch mode, which takes commands from external file
            for(int j = 0; j < sizeof(cmdRaw); j++){ //resets cmdRaw to empty to avoid leakage from previous command execution
                cmdRaw[j] = '\0';
            }

            if(fgets(cmdRaw, sizeof(cmdRaw), batchIn) == NULL){ //if there are no commands, the shell exits
                exit(1);
            }
            else{
                int increm = 0;
                while(cmdRaw[increm] != '\n' && cmdRaw[increm] != '\0' && cmdRaw[increm] != EOF){ // gets character count to make malloc'd trimmed string used for command parsing

                    increm++;
                }
                if(cmdRaw[increm] == '\n' || cmdRaw[increm] == EOF){
                    cmdRaw[increm] = '\0';
                }

                out = (char*) malloc(increm * sizeof(char)); // create malloc'd trimmed string for parsing
                strcpy(out, cmdRaw); // copy raw string to malloc'd string
                printf("\n\n");
            }
        }

        else{
            exit(1);
        }

        char * token = strtok(out, " "); // grab first word of command string

        // The following tests the first word of the command retrieved by strtok()
        // the first word signals what command the shell will perform
        // first the shell compares the first word against a list of bulid in commands
        // if the command is not among the build in commands, the shell will attempt to launch a forked child
        // (with piping and redirection accounted for) with an execution of a linux built-in command of the same name
        // If this fails, the shell throws back an error and the main loop iterates.

        if(token == NULL){ // nothing happens if no input is entered into the keyboard. this is to avoid a SEGFAULT. Program loops
            // nothing
        }
        else if(strcmp(token, "quit") == 0){  // exits shell by breaking loop and
            printf("\n\n%s\n", "Exiting...");
            exit(0);
        }
        else if(strcmp(token, "environ") == 0){ // lists environment variable. supports ouput redirection if next token is > or >>
            char * environment = getenv("PATH");
            token = strtok(NULL, " ");
            int saved_stdout = 0;
            int outFlag = 0;
            int errorFlag = 0;

            FILE * filep;


            if(token != NULL){
                if(strcmp(token, ">") == 0 || strcmp(token, ">>") == 0){
                    if(strcmp(token, ">") == 0){
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            errorFlag = 2;
                        }
                        else{
                            filep = fopen(token, "w");
                            if(filep != NULL){
                                if(fputs(environment, filep) == EOF){
                                    errorFlag = 2;
                                    fclose(filep);
                                }
                            }
                            else{
                                errorFlag = 2;
                            }
                        }
                    }
                    else if(strcmp(token, ">>") == 0){
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            errorFlag = 2;
                        }
                        else{
                            filep = fopen(token, "a");
                            if(filep != NULL){
                                if(fputs(environment, filep) == EOF){
                                    errorFlag = 2;
                                    fclose(filep);
                                }
                            }
                            else{
                                errorFlag = 2;
                            }
                        }
                    }
                }
                else{
                    errorFlag == 1;
                }
            }
            else{
                printf("\n\n%s\n", environment);
            }

            if(errorFlag == 2){
                printf("\n\n%s\n", "Error in Output Redirection");
            }
            if(errorFlag == 1){
                printf("\n\n%s\n", "Error in environ.");
            }
        }
        else if(strcmp(token, "path") == 0){
            char * oldPath = getenv("PATH");
            int errorFlag = 0;

            token = strtok(NULL, " ");
            if(token == NULL){
                errorFlag = 1;
            }
            else{
                strcat(oldPath, ";");
                strcat(oldPath, token);

                if(setenv("PATH", oldPath, 0) != 0){
                    printf("\n\nerror\n");
                }
            }
        }
        else if(strcmp(token, "pause") == 0){
            char c;

            printf("\n\n%s\n", "Paused. Press any key to continue...");

            c = getchar();
        }
        else if(strcmp(token, "cd") == 0){ // Changes working directory, and consequently the prompt, to specified directory. throws an error if fails or directory not fould
            token = strtok(NULL, " "); // next input

            if(chdir(token) == 0){
                printf("\n\n%s\n", "Directory changed!");
            }
            else{
                printf("\n\n%s\n", "Error");
            }
        }
        else if(strcmp(token, "echo") == 0){ // echos specified text out to terminal or output redirected file. supports multiple words
            int errorFlag = 0;
            int outputFlag = 0;
            int saved_stdout = 0;

            char ** filenameP = &outRedirectFilename;
            char ** tokP = &token;
            int * fileP = &outRedirect;
            int * stdoutP = &saved_stdout;
            int wordCount = 0;

            char raw[256];

            char * echoString;

            while(1){
                wordCount++;
                token = strtok(NULL, " ");
                if(token == NULL){
                    if(wordCount > 1){
                        break;
                    }
                    else{
                        printf("\n\n%s\n", "Error: Nothing to echo!");
                        errorFlag = 1;
                    }
                }
                else if(strcmp(token, ">") == 0 || strcmp(token, ">>") == 0){
                    if(strcmp(token, ">") == 0){
                        int outRedirResult = builtInOutputRedirect(tokP, filenameP, fileP, stdoutP);
                        if(outRedirResult == 0){
                            outputFlag = 1;
                            break;
                        }
                        else{
                            printf("\n\n%s\n", "Error in output redirection");
                            errorFlag = 1;
                            break;
                        }
                    }
                }
                else{
                    if(wordCount == 1){

                        strcpy(raw, token);
                    }
                    else{
                        strcat(raw, " ");
                        int length = strlen(token);
                        strncat(raw, token, length);
                    }
                }
            }
            if(errorFlag == 0){
                if(outputFlag == 0){
                    printf("\n\n");
                }
                printf("%s", raw);

                if(outputFlag == 1){
                    dup2(saved_stdout, 1);
                    close(saved_stdout);
                    close(outRedirect);
                }
                else{
                    printf("\n\n");
                }

            }

        }
        else if(strcmp(token, "help") == 0){
            int stdinNew = 0;
            int stdoutNew = 0;

            int errorFlag = 0;

            token = strtok(NULL, " ");

            char * args[64];
            char ** next = args;

            *next++ = "more";
            *next++ = NULL;

            stdinNew = open("readme", O_RDWR | O_CREAT, S_IRWXU | S_IRWXO);

            if(token == NULL){

                if(forkLogic(args, stdinNew, -1) != 0){
                    errorFlag = 1;

                }
            }
            else{
                if(strcmp(token, ">") == 0 || strcmp(token, ">>") == 0){
                    if(strcmp(token, ">") == 0){
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            errorFlag = 1;
                        }
                        else{
                            stdoutNew = open(token, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO);
                            if(stdoutNew == -1){
                                errorFlag = 1;
                            }
                            else{
                                if(forkLogic(args, stdinNew, stdoutNew) != 0){
                                    errorFlag = 1;
                                }
                            }
                        }
                    }
                }
            }

            if(errorFlag != 0){
                printf("\n\nError in help\n\n");
            }
        }
        else if(strcmp(token, "dir") == 0){ // lists items in a specified directory.
            int outRedirFlag = 0;           // if no directory is specified, it lists current directory
            int saved_stdout = 0;           // default output is terminal. supports output redirection with > and >>

            int errorFlag = 0;

            char * directoryName;
            char ** direcP = &directoryName;
            char ** tokP = &token;
            char ** filenameP = &outRedirectFilename;

            int * fileP = &outRedirect;
            int * savedP = &saved_stdout;

            token = strtok(NULL, " ");

            if(token == NULL || strcmp(token, ">") == 0 || strcmp(token, ">>") == 0){
                directoryName = curDirectory;
            }
            else{
                directoryName = token;
                token = strtok(NULL, " ");
            }


            if(token != NULL){
                if(strcmp(token, ">") == 0 || strcmp(token, ">>") == 0){
                    int outRedirResult = builtInOutputRedirect(tokP, filenameP, fileP, savedP);
                    if(outRedirResult == 0){
                        outRedirFlag = 1;
                    }
                    else if(outRedirResult < -1){
                        printf("\n\n%s\n", "Error in Output Redirection");
                        errorFlag = 1;
                    }
                }
            }
            if(errorFlag == 0){
                DIR *directory;
                struct dirent *s;
                directory = opendir(directoryName);
                if(directory == NULL){
                    printf("\n\n%s\n", "Error: Directory could not be found.");
                }
                else{
                    while((s = readdir(directory)) != NULL){
                        printf("%s\n", s->d_name);
                    }
                }

                if(outRedirFlag == 1){
                    dup2(saved_stdout, 1);
                    close(saved_stdout);
                    close(outRedirect);
                }

            }
        }

        else if(strcmp(token, "clr") == 0 || strcmp(token, "clear") == 0){ // clears screen
            printf("\e[1;1H\e[2J");
        }


        // The shell will now attempt to take the first word of input and launch a linux command
        // by forking a child and attemptint to find the command in directorys specified by the system PATH
        // The shell will also account for input redirection with <
        // output redirection with > for truncate and >> for append
        // and piping, with a second command on the right of the pipe being executed in a seperate child

        else{
            char *args[64]; // argument list for execvp command
            char **next = args; // pointer to shuffle through arguments



            char *pipeArgs[64]; // argument list for right-side pipe command
            char **nextPipe = pipeArgs; // pointer to shuffle through pipe arguments

            int pipeFlag = 0; // flag to signal if piping is used for this command
            int inputRedir = 0; // // flag to signal if input redirection is used
            int outputTruncate = 0; // flag to signal if output redirection truncate is used '>'
            int outputAppend = 0; // flag to signla if output redirection append is used '>>'
            int errorFlag = 0; // flag if error occurs

            *next++ = token; // first argument is the command itself
            token = strtok(NULL, " "); // cycle to next argument

            // the following loop parses through the rest of the string to gather arguments, or information about whether a redirect or pipe is
            // signalled by this command. The loop ends when either a redirect or pipe symbol is found, or until the input string has no more words
            // to parse

            while(token != NULL){ // test for next input as token cylces through words in command. ends when there are no more words
                if(strcmp(token, "|") == 0){ // if pipe is found, pipeflag is triggered
                    pipeFlag = 1;
                    break; // all command arguments occur before pipe marker, so loop is over
                }
                else if(strcmp(token, "<") == 0){ // if input redirection < is found, inputredir flag is triggered
                    inputRedir = 1;
                    break; // all command arguments occur before redirection mrkers, so loop is over
                }
                else if(strcmp(token, ">") == 0){ // if output redirection truncate > is found, outputTruncate is triggered
                    outputTruncate = 1;
                    break; // all command arguments occur before redirection markers, so loop is over.
                }
                else if(strcmp(token, ">>") == 0){ // if output redirection append >> is found, outputAppend is triggered
                    outputAppend = 1;
                    break; // all command arguments occur before redirection markers, so loop is over
                }

                // if token is none of these, token is another argument. add the argument to args list and keep looking
                *next++ = token;
                token = strtok(NULL, " ");
            }


            *next = NULL; // the last argument is null. this signals to execvp() that there are no more arguments

            if(pipeFlag == 1){ // pipe initialization logic
                token = strtok(NULL, " "); // check next input, which will be the first word after the | symbol

                if(token == NULL){ // having nothing after | symbol is invalid
                    errorFlag = 1;
                }
                else{
                    while(token != NULL){ // cycle through remaining words in command and add them to the pipe arguments
                        *nextPipe++ = token;  // end when there are no more words in command
                        token = strtok(NULL, " ");
                    }

                    *nextPipe = NULL; //last pipe argument is NULL due to execvp() needing it to signal end of arguemnts.
                }
            }

            // input redirection initialization

            else if(inputRedir == 1 && errorFlag == 0){
                char ** tokenP = &token; // pointer to tokenized word so passing to function works
                char ** inP = &inRedirectFilename; // pointer to input redirection filename string for passing to function

                int prepareInputResult = prepareInputRedirection(tokenP, inP); // parses rest of input string and sets input redirection filename
                if(prepareInputResult < 0){ // error has occured, therefore command invalid.
                    errorFlag = 1;
                }
                else if(prepareInputResult == 1){ // there is additional output redirection specified by this command, truncate
                    outputTruncate = 1;             // ex. ls < in.txt > out.txt
                }
                else if(prepareInputResult == 2){ // there is additional output redirection specified by this command
                    outputAppend = 1;               // ex. ls < in.txt >> out.txt
                }
            }

            // output redirection truncate initilization

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

            // output redirection append initialization

            else if(outputAppend == 1 && errorFlag == 0){
                char ** tokP = &token;
                char ** outP = &outRedirectFilename;
                int prepareOutputAppendResult = prepareOutputRedirection(tokP, outP);
                if(prepareOutputAppendResult < 0){
                    errorFlag = 1;
                }
                else if(prepareOutputAppendResult > 0){ // if additional input redirection is found
                    if(inputRedir == 1){  // if input redirection is already specified for this command
                        errorFlag = 1; // cannot have multiple input redirections
                    }
                    else{
                        char ** inP2 = &inRedirectFilename;
                        int inAfterAppendOutResult = prepareInputRedirection(tokP, inP2); // attempts to initialize this new input redirection
                        if(inAfterAppendOutResult < 0){ // if error
                            errorFlag = 1;
                        }
                        else if(inAfterAppendOutResult > 0){ // additional output redirection found
                            errorFlag = 1; // cannot have multiple output redirection for a singler command
                        }
                        else{
                            inputRedir = 1; // input redirection registered
                        }
                    }
                }
            }



            if(errorFlag == 0){ // if no errors, proceed with attempting to execute.

                // if this commnd involves piping
                if(pipeFlag == 1){
                    pipeLogic(args, pipeArgs);
                }


                else{
                    // ifthis command involve input redirection
                    if(inputRedir == 1){
                        inRedirect = open(inRedirectFilename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO); // opens or creates file specified by inRedirectFilename
                        if(inRedirect == -1){ // if error in file obtain
                            errorFlag = 1;
                        }
                    }

                    // if this command involves output redirection: append
                    if(outputAppend == 1 && errorFlag == 0){
                        outRedirect = open(outRedirectFilename, O_RDWR | O_CREAT | O_APPEND, S_IRWXU | S_IRWXO);
                        if(outRedirect == -1){
                            errorFlag = 2;
                        }
                    }

                    // if ths command involves output redirection: append
                    else if(outputTruncate == 1 && errorFlag == 0){
                        outRedirect = open(outRedirectFilename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO);
                        if(outRedirect == -1){
                            errorFlag = 2;
                        }
                    }

                    if(errorFlag == 0){ // if no erros

                    // forklogic() forks and executes based on parameters. -1 indicates that parameter is not set.
                    // forklogic() executes all possible execution cases with input redirection, output redirection, both, or neither
                    // forklogic() also handles sipmle commands with no redirection.

                    // the following are the cases that forklogic() can be run under.
                        if(inputRedir == 1 && outputTruncate == 0 && outputAppend == 0){ // Case: Execute with Input redirection, no output redirection for command
                            forkLogic(args, inRedirect, -1);
                        }
                        else if(inputRedir == 1 && (outputTruncate == 1 || outputAppend == 1)){ // Case: Execute with Input redirection and output redirection, truncate or append
                            forkLogic(args, inRedirect, outRedirect);
                        }
                        else if(inputRedir == 0 && outputTruncate == 0 && outputAppend == 0){ // Case: Execute with No redirection, args executed as they are with stdout set to terminal
                            forkLogic(args, -1, -1);
                        }
                        else{
                            forkLogic(args, -1, outRedirect); // Case: Execute with No input redirection, output redirection truncate or append.
                        }
                    }
                    else{
                        if(errorFlag == 1){ // error in input redirection
                            printf("\n\n%s\n", "Error in opening or creating input redirection file for this command.");
                        }
                        else if(errorFlag == 2){ // error in output redirection
                            printf("\n\n%s\n", "Error in opening or creating output redirection file for this command");
                        }
                    }
                }
            }
            else{
                // invalid command
                printf("\n\n%s\n", "Error: invalid input");
            }
        }
    // frees malloc'd command string sothat it can be realloc'd again in another loop, or to prevent memory leaks
    free(out);

    }

    return 0;
}

// prepareOnputRedirection takes over the parsing of token from the calling function. It handles determining the filename that output
//      will be redirected to. The filename is places int he same memorylocation as *filename, so that the calling function can use it.
//      Also checks for whether there is additional redirection, such as a input redirection, that the calling function must handle.

// return value 0 for no error
// return value -1 for general error
// return value 1 if additional onput redirection is found

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

// This is the output redirection handler for built-in shell functions. it takes in the token, which it continues to parse in lieu of the
// calling function. a pointer to the filename that will be discovered here, a pointer to the file pointer that will be opened, and a pointer
// to the saved default stdout value so the calling function can replace it manually

// this function does all the work of determining the filename, opening or creating the output file, saving the default stdout to a int in the calling
// function, and performs dup2 to switch stdout to the file specified and opened. The calling function must manually dup2 back to the original terminal output,
// which this function saves to the value that saved_stdout is pointed to.

//return value 0 -- no error
// return value -1 -- general error
// return value -2 -- no output redirection specified

int builtInOutputRedirect(char ** token, char ** filename, int * file, int * saved_stdout){
    int ret = 0;

    char * appendOrTruncate;

    if(strcmp(*token, ">") != 0 && strcmp(*token, ">>") != 0){
        *token = strtok(NULL, " ");
    }

    if(*token != NULL){
            if(strcmp(*token, ">") == 0 || strcmp(*token, ">>")){
                appendOrTruncate = *token;

                if(prepareOutputRedirection(token, filename) != 0){
                    ret = -1; //error
                }
                else{
                    if(strcmp(appendOrTruncate, ">") == 0){ // Truncate
                        *file = open(*filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO);
                        if(*file == -1){
                            ret = -1; //error
                        }
                        else{
                            *saved_stdout = dup(1);
                            dup2(*file, 1);
                        }
                    }
                    else if(strcmp(appendOrTruncate, ">>") == 0){ //Append
                        *file = open(*filename, O_RDWR | O_CREAT | O_APPEND, S_IRWXU | S_IRWXO);
                        if(*file == -1){
                            ret = -1; // error
                        }
                        else{
                            *saved_stdout = dup(1);
                            dup2(*file, 1);
                        }
                    }
                }

            }
        else{
            ret = -2; // no output redirection specified
        }
    }

    return ret;
}

// prepareInputRedirection takes over the parsing of token from the calling function. It handles determining the filename that output
//      will be redirected to. The filename is places int he same memorylocation as *filename, so that the calling function can use it.
//      Also checks for whether there is additional redirection, such as a input redirection, that the calling function must handle.

// return value 0 for no error
// return value -1 for general error
// return value 1 if additional input redirection is found

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

// forklogic() contains all the logic for forking and executing a non-piped command, while accounting for input and output redirection.
// args[] is the arugment list generated by the calling function.
//      inRedirect is a file pointer to a file used for input redirection. if -1 is passed
//       to inRedirect, there is no input redirection for this command

//      outRedirect is a file pointer to a file used for output redirection. if -1 is passed
//       to outRedirect, there is no output redirection for this command.

// The child produced by the fork takes the arguments and output/input redirection settings, makes adjustsments to stdin and stdout for input and output redirection respectively,
//  and executes the command using parameters. Command should execute and exit flawlessly, otherwise and error is thrown.

// the parent produced by the fork waits for its child to finish executing, closes file pointers for input and output redirection if there are any, and then
//  returns control flow back to calling function.

// return value 0 -- no error
// return value 1 -- general error

int forkLogic(char* args[], int inRedirect, int outRedirect){
    int pid; // process id
    int ret = 0;// return value

    int inFlag = 0; // input redirection flag
    int outFlag = 0; // output redirection flag

    pid = fork(); // this is where a process is forked. the child of which will run the command specified and the parent will wait before returnign to teminal

    if(pid < 0){ // error with forking
        printf("\n\nForking error!");
        ret = 1;
    }
    else if(pid == 0){ // CHILD PROCESS
        if(inRedirect != -1){ // output and input redirection handlers
            int check1 = dup2(inRedirect, 0); // sets stdIN to file pointed to by inRedirect
            inFlag = 1;
        }
        if(outRedirect != -1){
            int check2 = dup2(outRedirect, 1); // sets STDOUT to file pointed to by outRedirect
            outFlag = 1;
        }

        int execute = execvp(args[0], args); // executes command with arguments. should not return if successful
        if(execute != 0){ // returns if error and exits child process
            printf("\n\nExecution Error in fork\n\n");
            exit(0);
        }
    }
    else{ // PARENT PROCESS
        wait(NULL); // waits for child process to exit
        if(inFlag == 1){
            close(inRedirect); // closes file pointed to by inRedirect if input redirection happened during this command
        }
        if(outFlag == 1){
            close(outRedirect); // closes file pointed to by outRedirect if output redirection happened during this command
        }
    }

    return ret;
}

// pipeLogic() handles all pipe related execution. It takes in two sets of commands and arguemnts, one derived from the left hand of the | symbol,
//      and one derived from the right hand.

// a file descriptor table is created and put through pipe()

// The process forks and creates child 1. This child closes its STDIN and sets its STDOUT to the pipe, so that it can become the input to child 2 process.
//    Child 1 then executes left hand command according to args[]


// The parent then forks again and creates child 2. This child closes its STDOUT and sets its STDIN to the pipe, so that it can take in input from child 1 process.
//    Child 2 then executes right hand commands according to pipeArgs[]

// The parent then closes both ends of the fd table and waits for child 1 and child 2 to exit. Then, parent reliquishes control back to calling function.

// return value 0 -- no error
// return value 1 -- general error


int pipeLogic(char* args[], char* pipeArgs[]){
    int pid; // left pipe process id
    int pid2; // right pipe process id
    int fd[2]; // file descriptor table for pipe
    int ret = 0; // return value

    if(pipe(fd) < 0){ // pipes file descriptor table
        printf("Error in piping");
        ret = 1;
    }
    else{
        pid = fork(); // fork for left hand commands
        if(pid < 0){
            printf("Forking error!");
            ret = 1;
        }

        else if(pid == 0){ // CHILD
            close(fd[0]); // close stdIN, set stdout to pipe
            dup2(fd[1], 1);
            close(fd[1]);

            int execute = execvp(args[0], args); // execute command with arguments. shouldnt return if all goes wel
            if(execute != 0){ // all does not go well
                printf("\n\n%s\n", "-1 error");
                exit(0);
            }
        }
        else{ // PARENT
            pid2 = fork(); // fork again for right hand pipe command

            if(pid2 < 0){ // fork error
                printf("fork error");
                ret = 1;
            }
            else if(pid2 == 0){ // NEW CHILD  right hand pipe command
                close(fd[1]); // close STDOUT and set STDIN to output of first child
                dup2(fd[0], 0);
                close(fd[0]);
                int execute2 = execvp(pipeArgs[0], pipeArgs); // execute right hand pipe arguements. shouldn't return if all goes well
                if(execute2 < 0){ // all does not go well
                    printf("-1 error");
                    exit(0);
                }
            }
            else{ // NEW PARENT , also original parent as well
                close(fd[0]); // close both sides of pipe fd table
                close(fd[1]);
                waitpid(pid, NULL, WCONTINUED); // wait for child 1, left hand pipe command, to finish executing
                waitpid(pid2, NULL, WCONTINUED); // wait for child 2, right hand pipe command, to finish executing
            }
        }

    }
    return ret;
}

