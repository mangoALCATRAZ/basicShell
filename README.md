#basic Shell

This is a basic shell that is compatible with Ubuntu.

The shell begins at the current directory that it is launched from. It provides a prompt that identifies it as 'myshell' and prints the current directory along with a '>' charcter. 



At this point, the user can type in a command.
A command, with a few exceptions, generally takes the following structure:

"command <arg1> <arg2> ... <lastArg>"

	with 'command' being the function or program that is to be executed, and <arg1> through <lastArg> being arguments that will affect the functioning of command.


 



The Shell has several built-in commands:

'clear' or 'clr' -- erases all text on screen except for an input prompt

'cd <directory>' -- changes the current open directory to <directory>, which also changes all subsequent prompts.

'dir <directory>' -- prints contents of <directory> to standard output. if <directory> is not specified, this prints contents of current directory. 

'environ' -- prints PATH enviornment variables for this shell instance. These are a semicolon separated list of directories that the shell will use to search for command executables. 

'echo <string1> <string2> ... <etc>' -- prints <string1>, <string 2>, ... up to <etc> to standard output. Can take many arguments for input. Must have at least one whitespace-seperated word to work.

'help' -- Displays this manual. The one you're reading right now!

'pause' -- Pauses execution of the shell until the RETURN key is pressed.

'quit' -- exits the shell.




Any other 'command' will be executed by the shell as an 'external command'. This means that the shell will attempt to find the function or program the command references within the Linux OS environment. In order to to this, the shell attempts to find the 'command' within directories specified by the shell's PATH environment. 

For example: If the user were to input the command

'ls'

'ls' is not a built-in command for this shell. Therefore, the shell assumes that it is an external command. The shell will create a new process and attempt to find a program called 'ls' in the directories specified by PATH. If it finds it, the shell will execute the program, thus executing the external command.




External commands also support piping. Piping refers to running two commands, with the Standard Output for the first command becoming the Standard Input for the second. 

Piping for this shell IS ONLY SUPPORTED BY EXTERNAL COMMANDS. BUILT-IN COMMANDS DO NOT SUPPORT PIPING.

Piping syntax looks like the following:

'command1 <arg1> <arg2> ... <lastArg> | command2 <arg1> <arg2> ... <lastArg>'

With command1 being the process that will feed its output, instead of back to the terminal, to the input of command2. 



In addition, all external commands, as well as a few select built-in commands, support I/O Redirection. I/O Redirection, or Input/Output Redirection, redirecets either the input, output, or both of a command to an external file kept somewhere on the disk. 

The following symbols represent I/O Redirection in a command:
'<' - input redirection
'>' - output redirection where the contents of the file are truncated to make way for new data
'>>' - output redirection where the new data is appended to the end of the file contents.




The syntax for input redirection looks like the following:
'command <arg1> <arg2> ... <lastArg> < in.txt'

This means that the Standard Input Stream of data that would normally go into 'command' is now the data from in.txt.




The syntax for output redirection looks like the following:
'command <arg1> <arg2> ... <lastArg> > out.txt' -- for truncate
or
'command <arg1> <arg2> ... <lastArg> >> out.txt' -- for append

This mans that the Standard Output Stream of data, which would normally go back to the terminal, now goes to out.txt. In the former's case, the data in out.txt is overwritten. In the latter, the new data is appended onto the end of out.txt


In addition, the following built-in command support output redirection for both truncate and append. These built-in commands DO NOT support input redirection or piping:

'dir'
'environ'
'echo'
'help'


