/*
 * Programmer: Valerie Lam
 * Class: CIS 3207 - Section 001
 * Due Date: October 23rd, 2020
 * Assignment: Project 2 - Devloping a Linux Shell Type Program
 * Version: 3
 * Description: Create a Linux Shell - The goal for the final submission is successful parsing, I/O redirection
 *					forking, execvp use, piping, and background execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#define CD 1
#define CLR 2
#define QUIT 3
#define DIRECT 4
#define ENVIRON 5
#define ECHO 6
#define PAUSE 7
#define HELP 8


//This is a structure created to simplify the organization of the command, or first parsed token, and the arguments
//which are parsed next via strtok.
struct stringOrganizer {
        //Note that the space for these two pointers cannot be allocated here because structures only handle variables.
	char *command;
        char *argument[10]; //Array of strings, each string is 30 characters long.
};

//Function prototypes as follows.
int* inputOutputFiles(struct stringOrganizer input);
void fixStream(int stdin, int stdout, int* fp);
void execute(struct stringOrganizer input, int bkrdFlag);
void cd(char *array[10]);
void clear(void);
void quit(void);
void dir(char *array[10]);
void environmentStrings(void);
void echoCopy(char *array[10]);
void enterPause(void);
void help(char *array[10]);
void isPipe(struct stringOrganizer input, int pipeLocation);

int main(int argc, char *argv[]){
	int i, batch_validity = -1, fileFlag = -1, pipeFlag = -1, pipeLocation = -1, bkrdFlag = -1, bkrdLocation = -1;
	char *buffer = NULL;
	char *token = (char*) malloc(100);
	FILE *stream = NULL;

	//Memory allocation of the two input pointer variables.
	struct stringOrganizer input;
	input.command = (char*) malloc(20);

	for (i = 0; i < 10; i++){
		input.argument[i] = malloc(10 * sizeof(char*));
	}

	//The purpose of this block of code is to check if a batch file is needed, and if it is, set validity to 1.
	if (argc > 1){ //If there is more than one argument (./shell) in the cmdline...
		batch_validity = 1;
	} else batch_validity = 0;


	//The following block of code adjusts the stream to the batch file if the batch validity is true.
	if(batch_validity == 1){
		stream = fopen(argv[1], "r");
		if(stream == NULL){
			puts("ERROR: FILE NOT FOUND!");
			exit(EXIT_FAILURE);
		}
	} else {
		stream = stdin;
	}

	//This block of code prints the prompt, gets the next line from either file or standard input.
	//Uses strtok to break down the string and organize it using the structure.
	//Then checks to see if I/O redirection, piping, or background execution is necessary.
	//Near the end, it calls fixStream to adjust the streams back to standard input and output, in the event
	//that I/O redirection adjusted them. It will do this every time regardless.
	//Then it frees the allocated strings so that they are able to reused again.
	while(1){
		printf("myshell> ");

		//Scan in the command.
		size_t inputLen = 0;
		getline(&buffer, &inputLen, stream);

		//Remove the trailing newline character.
		for(i = 0; i < 100; i++){
			if(buffer[i] == '\n')
				buffer[i] = '\0';
		}

		//Collect the first argument, which should be the command.
		token = strtok(buffer," ");

		//Check to see if the command is equal to exit.
		if(strcasecmp(token, "exit") == 0)
			break;
		else{
			i = 0;
			strcpy(input.command, token);

			token = strtok(NULL, " ");
			while (token != NULL){
				strcpy(input.argument[i++], token);
				token = strtok(NULL, " ");
			}


			//Check to see if I/O redirection is necessary.
			for(i = 0; i < 10; i++){
				if(strcmp(input.argument[i], ">") == 0){
					fileFlag = 1;
					break;
				} else if(strcmp(input.argument[i], "<") == 0){
					fileFlag = 1;
					break;
				} else if(strcmp(input.argument[i], ">>") == 0){
					fileFlag = 1;
					break;
				} else fileFlag = 0;
			}

			//Check to see if piping is ncessary.
			for(i = 0; i < 10; i++){
				if(strcmp(input.argument[i], "|") == 0){
					pipeFlag = 1;
					pipeLocation = i;
					break;
				} else pipeFlag = 0;
			}

			//Check to see if background execution is necessary.
			for(i = 0; i < 10; i++){
                                if(strcmp(input.argument[i], "&") == 0){
                                        bkrdFlag = 1;
					bkrdLocation = i;
                                        break;
                                } else bkrdFlag = 0;
                        }
			if (bkrdFlag > 1){
				input.argument[bkrdLocation] = NULL;
			}

			//Save a copy of the original stdin and stdout streams.
			int stdin_copy = dup(STDIN_FILENO);
                       	int stdout_copy = dup(STDOUT_FILENO);

			//Adjust the streams if I/O redirection is necessary.
			int *fp;
			if (fileFlag > 0){
				fp = inputOutputFiles(input);
			}

			//Pipe if necessary, otherwise execute as normal.
			if(pipeFlag > 0){
				isPipe(input, pipeLocation);
			} else {
			//Call the function execute while passing in the structure containing the command and args.
				execute(input, bkrdFlag);
			}

			//Enter the fixStream function to reset stdin and stdout.
			fixStream(stdin_copy, stdout_copy, fp);

			//The purpose of this is to empty the input.argument array for the consecutive runs.
			//NOTE: Attempt was made to make every element an empty string, that caused segmentation fault.
			for (i = 0; i < 10; i++){
				free(input.argument[i]);
         	        	input.argument[i] = malloc(10 * sizeof(char*));

        		}

			//Free the allocated buffer.
			free(buffer);
			buffer = NULL;
		}
	}
	return EXIT_SUCCESS;
}

//This function takes in the structure that contains the original command and tokenized array arguments.
//It also takes in the location of the pipe so that the strings can be split based on the location of the pipe.
void isPipe(struct stringOrganizer input, int pipeLocation){
	puts("In piping function");
	pid_t childPIDOne, childPIDTwo;
	int builtOne = -1, builtTwo = -1, i, x;

	//Create the character arrays to hold the split strings.
	char *firstCommand[10];
	char *secondCommand[10];
	for (i = 0; i < 10; i++){
                firstCommand[i] = malloc(10 * sizeof(char*));
		secondCommand[i] = malloc(10 * sizeof(char*));
        }

	//Separate the command and arguments before the pipe.
	strcpy(firstCommand[0], input.command);
	i = 0;
	while(i < pipeLocation){
		//Note: the firstCommand[0] holds the first command, and 1 onward contain firstCommand arguments.
		strcpy(firstCommand[i+1], input.argument[i]);
		i++;
	}
	i++; //This addition is meant to increment the location index past the pipe.

	//Separate the command and arguments after the pipe.
	x = 0;  //Note: there is a need for x because we want the second split command array to begin at zero
		//even though we are still traversing the entirety of the original array.
	while(i < 10){
		strcpy(secondCommand[x], input.argument[i]);
		x++;
		i++;
	}

	int pfds[2], status = 0;
	if (pipe(pfds) == 0){
		if ((childPIDOne = fork()) == 0){
			dup2(pfds[1], 1);
			close(pfds[0]);
			close(pfds[1]);

				//The purpose of this code is to locate empty strings and change them to null
				//in order to be formatted correctly for execvp().
	                        i = 0;
				while(i < 10){
                                	if(strcmp(firstCommand[i], "") == 0){
                                        	//Insert a null pointer.
                                        	firstCommand[i] = NULL;
					}
					i++;
                                }

				if(execvp(firstCommand[0], firstCommand) == -1){
        	                        puts("Execvp failed.");
	                        	exit(1);
				}
			exit(0);
		} else {
			if((childPIDTwo = fork()) == 0){
				dup2(pfds[0], 0);
				close(pfds[1]);
				close(pfds[0]);

						i = 0;
						while(i < 10){
							if(strcmp(secondCommand[i], "") == 0){
								secondCommand[i] = NULL;
							}
						i++;
						}

                                		if(execvp(secondCommand[0], secondCommand) == -1){
                                        		puts("Execvp failed.");
                                        		exit(1);
                                		}
				exit(0);
			} else {
				int statusTwo;

				close(pfds[0]);
				close(pfds[1]);

				waitpid(childPIDOne, &status, 0);
				waitpid(childPIDTwo, &statusTwo, 0);
			}
		}
	}
}

//This function takes in the structure containing the original command and argument array.
//The purpose of this function is to adjust the input/output streams based on the presence of redirection symbols.
int* inputOutputFiles(struct stringOrganizer input){
	int *fp = (int*)malloc(sizeof(int)*2);
	fp[0] = 0;
	fp[1] = 1;
	int file_desc, i;

	//This for loop will traverse all the arguments.
        for(i = 0; i < 10; i++){
		//This if statement will locate the input redirection symbol and will adjust the input stream.
                if(strcmp(input.argument[i], "<") == 0){
			fp[0] = open(input.argument[i+1], O_RDONLY); //Opens a file that is read only.
			if(dup2(fp[0], STDIN_FILENO) < 0){ //After this line, newstdin has a file descriptor for the specified file.
				puts("Unable to duplicate file descriptor.");
				exit(EXIT_FAILURE);
			} else {
				close(fp[0]);
			}

		//This if statement will locate the output redirection symbol and will adjust the output stream.
		//If the output file already exists, it will overwrite it.
                } else if(strcmp(input.argument[i], ">") == 0){
			fp[1] = open(input.argument[i+1],O_WRONLY|O_TRUNC|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
			if(dup2(fp[1], STDOUT_FILENO) < 0){
				puts("Unable to duplicate file descriptor.");
				exit(EXIT_FAILURE);
			} else {
				close(fp[1]);
			}

		//This if statement will locate the double output redirection symbol and will adjust the output stream.
		//However, if the outpute file already exists, it will append text instead of overwrite.
		} else if(strncmp(input.argument[i], ">>", 2) == 0){
			fp[1] = open(input.argument[i+1],O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
                        if(dup2(fp[1], STDOUT_FILENO) < 0){
                                puts("Unable to duplicate file descriptor.");
                                exit(EXIT_FAILURE);
                        } else {
                                close(fp[1]);
                        }
		}
        }
	return fp;
}

//The purpose of this function is to readjust stream to standard input and output.
void fixStream(int stdin_copy, int stdout_copy, int *fp){
	dup2(stdin_copy, STDIN_FILENO);
	dup2(stdout_copy, STDOUT_FILENO);
	close(stdin_copy);
	close(stdout_copy);
}

//This function will identify if the command is a built in command or not.
//If it is, it will run that command by calling the function. Otherwise it will reformat the input.argument string
//so that it will work with execvp. If background execution is enabled, then the child will run the background.
void execute(struct stringOrganizer input, int bkrdFlag){
	pid_t PID;
	int status = 0;
	int i, whatisEx = 999;

	//The purpose of the if statements is to identify whether the command is built in or not.
	if(strcmp(input.command, "cd") == 0){
		cd(input.argument);
	} else if(strcmp(input.command, "clr") == 0){
		clear();
	} else if(strcasecmp(input.command, "quit") == 0){
		quit();
	} else if(strcmp(input.command, "dir") == 0){
		dir(input.argument);
	} else if(strcmp(input.command, "environ") == 0){
		environmentStrings();
	} else if(strcmp(input.command, "echo") == 0){
		echoCopy(input.argument);
	} else if(strcmp(input.command, "pause") == 0){
		enterPause();
	} else if(strcmp(input.command, "help") == 0){
		help(input.argument);
	} else {
		PID = fork();
		if(PID < 0) {
			printf("Forking child process unsuccessful.\n");
			exit(1);

		//If the PID is equal to zero, this means it is the child process.
		} else if(PID == 0){
			pid_t val = getpid();

        //The purpose of this bit of code is to move shift all strings in the array to the right by one.
        //This is because the second argument to execvp must be an array of all commandline arguments, including the command.
                        for(i = 9; i > 0; i--){
                                input.argument[i] = input.argument[i-1];
                        }
                        input.argument[0] = input.command;

			i = 0;
			while(i < 10){

				//NOTE: There is a difference between a pointer to an empty string vs. a null pointer.
				if(strcmp(input.argument[i], "") == 0){
					//Insert a null pointer.
					input.argument[i] = NULL;
        			}
				i++;
			}


			if(execvp(input.command, input.argument) == -1)
				puts("Execvp failed.");
			exit(0);
	//The only time this else block would be accessed is if it is the parent process, with a PID greater than, but not equal to 0.
		} else if (PID > 0){
			if(bkrdFlag == 0){
				int status;
				waitpid(PID, &status, 0);
			}
                } else {
			perror("Fork failure.");
		}
	}
}

//This function is meant to replicate the cd function.
void cd(char *array[10]){
	char currentDir[256];
	int didItWork = chdir(array[0]);
	if (didItWork == 0){
		if(getcwd(currentDir, sizeof(currentDir)) != NULL)
			puts(currentDir);
		else perror("Failure with getcwd()");
	} else {
		puts("Failure with chdir()");
		return;
	}
}


//This function will clear the screen.
void clear(){
	printf("\033[H\033[2J");
}

//This function will quit the program.
void quit(){
	puts("Exiting shell.");
	exit(0);
}

//This function is meant to replicate the dir function.
void dir(char *array[10]){
	DIR *open;
	struct dirent *dir;
	open = opendir(array[0]);
	if(open){
		while((dir = readdir(open)) != NULL){
			printf("%s ", dir->d_name);
		}
	} else {
		puts("Unable to read directory.");
	}
	closedir(open);
}

//This function will print three environment strings, PATH, HOME, and ROOT.
void environmentStrings(){
	printf("PATH: %s\n", getenv("PATH"));
	printf("HOME: %s\n", getenv("HOME"));
	printf("ROOT: %s\n", getenv("ROOT"));
}

//This function echos whatever the user typed in, besides the command.
void echoCopy(char *array[10]){
	int i = 0;
	const char null[5] = {'\0'};
	while(strcmp(array[i], null) != 0){
		printf("%s ", array[i++]);
	}
	puts("");
}

//This function will pause the program until the enter key is pressed.
void enterPause(){
	char typed;
	puts("Program is paused until user types enter key.");
	do {
		typed = getchar();
	} while(typed != '\n');
}

//This function will print the readme.md file for this lab assignment.
void help(char *array[10]){
	int n = 512;
	char buffer[n];
	FILE *fp;
	fp = fopen("readme_doc", "r");
	while(fgets(buffer, n, fp) != NULL)
		printf("%s", buffer);
}
