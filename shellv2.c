/*
 * Programmer: Valerie Lam
 * Class: CIS 3207 - Section 001
 * Due Date: October 17th, 2020
 * Assignment: Project 2 - Devloping a Linux Shell Type Program
 * Version: 2
 * Description: Create a Linux Shell - The goal for the second submission is built-in commands, file redirection, and piping.
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
//#include "input.txt"

int originalInput, originalOutput;

struct stringOrganizer {
        //Note that the space for these two pointers cannot be allocated here because structures only handle variables.
	char *command;
        char *argument[10]; //Array of strings, each string is 30 characters long.


/////////////////////////////////////////////////////////////////
	//Include variables to indicate whether there is piping. (TRUE OR FALSE)
	//REDIRECTION (TRUE OR FALSE)
	//BACKGROUND EXECUTION (TRUE OR FALSE)
/////////////////////////////////////////////////////////////////
};

int* inputOutputFiles(struct stringOrganizer input);
void fixStream(int stdin, int stdout, int* fp);
void execute(struct stringOrganizer input, int *fp);
void cd(struct stringOrganizer input);
void clear(void);
void quit(void);
void dir(struct stringOrganizer input);
void environmentStrings(void);
void echoCopy(struct stringOrganizer input);
void enterPause(void);
void help(struct stringOrganizer input);

//File redirection not yet implemented. Need to use dups()
//Piping not yet implemented.
//Background execution not yet implemented.

int main(int argc, char *argv[]){
	int i, batch_validity = -1, fileFlag = -1, x;
	char *buffer = NULL;
	char *token = (char*) malloc(100);
	FILE *stream = NULL;

	//Memory allocation of the two input pointer variables.
	struct stringOrganizer input;
	input.command = (char*) malloc(20);

	for (i = 0; i < 10; i++){
		input.argument[i] = malloc(10 * sizeof(char*));
	}

	//The purpose of this segment of code is to identify if the shell was involved with batch0.
	//If batch_validity = 1, the shell should run in batch mode, otherwise run in interactive mode.

////////////////////////////////////////////////////////////////////////////////
	//IF THERE IS MORE THAN ONE ARGUMENT: IT IS A BATCH FILE. READ FROM IT.
////////////////////////////////////////////////////////////////////////////////

	if (argc > 1){ //If there is more than one argument (./shell) in the cmdline...
		batch_validity = 1;
	} else batch_validity = 0;

/*
	//The purpose of this segment of code is to initalize all characters of the buffer array to null.
	for(i = 0; i < 100; i++){
		buffer[i] = '\0';
	}
*/
//	if(batch_validity = 0){

	if(batch_validity == 1){
		stream = fopen(argv[1], "r");
		if(stream == NULL){
			puts("ERROR: FILE NOT FOUND!");
			exit(EXIT_FAILURE);
		}
	} else {
		stream = stdin;
	}

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
//		printf("1: %s\n", token);
//		puts("Successfully collects command");

			//Check to see if the command is equal to exit.
			if(strcasecmp(token, "exit") == 0)
				break;
			else{
				i = 0;
//	puts("Before storing command");
				strcpy(input.command, token);
//			puts("Stores command");

				token = strtok(NULL, " ");
//			printf("2: %s\n", token);
				while (token != NULL){
					printf("The value of i in tokenizing loop is %d\n", i);
					printf("The value of arg %d token is %s\n", i, token);
					strcpy(input.argument[i++], token); //Seg fault with this line.
					printf("Get the token within loop\n");
					token = strtok(NULL, " ");
//				printf("3: %s\n", token);
				}
//			puts("Tokenizes remaining array");

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

//			printf("The final value of fileFlag is: %d\n", fileFlag);

				//Save a copy of the original stdin and stdout streams.
				int stdin_copy = dup(STDIN_FILENO);
                        	int stdout_copy = dup(STDOUT_FILENO);

				for(i = 0; i < 10; i++){
					printf("%s.", input.argument[i]);
				}
				puts("");

				//Note: pointer space not allocated.
				int *fp;
				if (fileFlag > 0){
					fp = inputOutputFiles(input);
				}

				//Call the function execute while passing in the structure containing the command and args.
				execute(input, fp);
				//Enter the fixStream function to reset stdin and stdout
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

int* inputOutputFiles(struct stringOrganizer input){
	int *fp = (int*)malloc(sizeof(int)*2);
	fp[0] = 0;
	fp[1] = 1;
	int file_desc, i;
        for(i = 0; i < 10; i++){
                if(strcmp(input.argument[i], "<") == 0){
			printf("Shell: reading from an input file\n");

//returns two file numbers, array of integers
//First one is input, second is output
//fp is equal to 0 1 by default, aka using stdinput & stdoutput
//fp should be changed from 0 to inputfile if there is input

			fp[0] = open(input.argument[i+1], O_RDONLY);
			puts("Finished open");
			printf("The value of fp[0] is %d\n", fp[0]);
			//DUP2(NEW,OLD) -- THEN CLOSE OLD STREAM
			if(dup2(fp[0], STDIN_FILENO) < 0){
				puts("Unable to duplicate file descriptor.");
				exit(EXIT_FAILURE);
			} else {
				puts("Finished dup2");
				close(fp[0]);
				printf("Successful input redirection.\n");
			}
/*
			//Opens a file that is read only.
			//After this line, newstdin has a file descriptor for the specified file.
			fp[0] = open(input.argument[i+1], O_RDONLY);
			//The close statement "closes stdin" which means user input is not read.
			close(0);
			//Dup creates a copy of a file descriptor, and uses the lowest numbered unused
			//descriptor for the new descriptor.

			dup(fp[0]);
\//			if(file_desc < 0)
//				puts("Error with dup()");
			close(fp[0]);
*/

                } else if(strcmp(input.argument[i], ">") == 0){
			printf("Shell: printing to an output file\n");
			fp[1] = open(input.argument[i+1],O_WRONLY|O_TRUNC|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
			printf("Finished open\n");
			printf("The value of f[1] is %d\n", fp[1]);
			if(dup2(fp[1], STDOUT_FILENO) < 0){
				puts("Unable to duplicate file descriptor.");
				exit(EXIT_FAILURE);
			} else {
				puts("Finished dup2");
				close(fp[1]);
				printf("Successful output redirection.\n");
			}
/*			close(1);
			file_desc = dup(fp[1]);
//			if(file_desc < 0)
//				puts("Error with dup()");
			close(fp[1]);
*/		} else if(strncmp(input.argument[i], ">>", 2) == 0){
			printf("Shell: appends to an output file\n");
			fp[1] = open(input.argument[i+1],O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
                        printf("Finished open\n");
                        printf("The value of f[1] is %d\n", fp[1]);
                        if(dup2(fp[1], STDOUT_FILENO) < 0){
                                puts("Unable to duplicate file descriptor.");
                                exit(EXIT_FAILURE);
                        } else {
                                puts("Finished dup2");
                                close(fp[1]);
                                printf("Successful output append/redirection.\n");
                        }
		}
        }
	return fp;
}

void fixStream(int stdin_copy, int stdout_copy, int *fp){
	//Close the files -- must be passed in. Original use in inputOutputFiles.
	//close(*(fp + 0));
	dup2(stdin_copy, STDIN_FILENO);
	dup2(stdout_copy, STDOUT_FILENO);
	close(stdin_copy);
	close(stdout_copy);
}

/*
int isItBuiltIn(struct stringOrganizer input){
	if(strcmp(input.command, "cd") == 0){
		return 1;
        } else if(strcmp(input.command, "clr") == 0){
		return 1;
        } else if(strcasecmp(input.command, "quit") == 0){
		return 1;
        } else if(strcmp(input.command, "dir") == 0){
		return 1;
        } else if(strcmp(input.command, "environ") == 0){
		return 1;
	} else if(strcmp(input.command, "echo") == 0){
		return 1;
        } else if(strcmp(input.command, "pause") == 0){
		return 1;
        } else if(strcmp(input.command, "help") == 0){
		return 1;
	} else return 0;
}
*/

void execute(struct stringOrganizer input, int *fp){
	puts("In execute function");
	pid_t PID;
	int status = 0;
	int i, whatisEx = 999;
/*	int builtIn = -1;

	builtIn = isItBuiltIn(input);
	if (builtIn == 1){
		cmdExe();
	}
*/
	if(strcmp(input.command, "cd") == 0){
		cd(input);
	} else if(strcmp(input.command, "clr") == 0){
		clear();
	} else if(strcasecmp(input.command, "quit") == 0){
		quit();
	} else if(strcmp(input.command, "dir") == 0){
		dir(input);
	} else if(strcmp(input.command, "environ") == 0){
		environmentStrings();
	} else if(strcmp(input.command, "echo") == 0){
		echoCopy(input);
	} else if(strcmp(input.command, "pause") == 0){
		enterPause();
	} else if(strcmp(input.command, "help") == 0){
		help(input);
	} else {
		//FILE POINTERS MUST BE TAKEN IN HERE
		PID = fork();
		if(PID < 0) {
			printf("Forking child process unsuccessful.\n");
			exit(1);

		//If the PID is equal to zero, this means it is the child process.
		} else if(PID == 0){
			puts("Child process");
			pid_t val = getpid();
			//Rewrite stdin with fp[0], aka whatever file name.
			//NOTE: This is only done in the child process and will not need to be "cleaned up" later.
//			dup2(fp[0], STDIN_FILENO);
			printf("Child PID is %d\n", val);
//int i;
//const char null[5] = {'\0'};

			//puts(input.command);
			int i;
			const char null[5] = {'\0'};

        //The purpose of this bit of code is to move shift all strings in the array to the right by one.
        //This is because the second argument to execvp must be an array of all commandline arguments, including the command.
                        for(i = 9; i > 0; i--){
                                input.argument[i] = input.argument[i-1];
                        }
                        input.argument[0] = input.command;

			i = 0;
			printf("\nThe strings in input.argument are:\n");
			while(i < 10){

				//NOTE: There is a difference between a pointer to an empty string vs. a null pointer.
				if(strcmp(input.argument[i], "") == 0){
					//Insert a null pointer.
					input.argument[i] = NULL;
				} else {
                			printf("%s.", input.argument[i]);
        			}
				i++;
			}
			puts("");



			printf("The value of input.command is %s\n", input.command);

			if(execvp(input.command, input.argument) == -1)
				puts("Execvp failed.");
//				printf(stderr, "execvp() failed. errno = %d", errno);
			exit(0);
	//The only time this else block would be accessed is if it is the parent process, with a PID greater than, but not equal to 0.
		} else if (PID > 0){
			printf("Waiting on child process...\n");
//			while(wait(&status) != PID)
//				;
			int num = wait(&status);
			printf("Value returned from wait: %d\n", num);
			printf("Finished waiting for child process to terminate.\n");
                } else {
			perror("Fork failure.");
		}
	}
}

void cd(struct stringOrganizer input){
	puts("In cd function");
	char currentDir[256];
	int didItWork = chdir(input.argument[0]);
	if (didItWork == 0){
		if(getcwd(currentDir, sizeof(currentDir)) != NULL)
			puts(currentDir);
		else perror("Failure with getcwd()");
	} else {
		puts("Failure with chdir()");
		return;
	}
}

void clear(){
	printf("\033[H\033[2J");
}

void quit(){
	puts("Exiting shell.");
	exit(0);
}

void dir(struct stringOrganizer input){
	puts("In dir function");
	DIR *open;
	struct dirent *dir;
	open = opendir(input.argument[0]);
	if(open){
		while((dir = readdir(open)) != NULL){
			printf("%s ", dir->d_name);
		}
		closedir(open);
	}
}

void environmentStrings(){
	puts("In environ function");
	printf("PATH: %s\n", getenv("PATH"));
	printf("HOME: %s\n", getenv("HOME"));
	printf("ROOT: %s\n", getenv("ROOT"));
}

void echoCopy(struct stringOrganizer input){
	int i = 0;
	const char null[5] = {'\0'};
	while(strcmp(input.argument[i], null) != 0){
		printf("%s ", input.argument[i++]);
	}
	puts("");
}

void enterPause(){
	char typed;
	puts("In pause function");
	do {
		typed = getchar();
	} while(typed != '\n');
}

//Is this correct? Do I need to use the more filter via execvp.
void help(struct stringOrganizer input){
	int n = 512;
	char buffer[n];
	FILE *fp;
	if(strcmp(input.argument[1], "cd") == 0){
		puts("Changes the shell working directory.");
        } else if(strcmp(input.argument[1], "clr") == 0){
		puts("Clears the screen.");
        } else if(strcasecmp(input.argument[1], "quit") == 0){
		puts("Quits the shell program.");
        } else if(strcmp(input.argument[1], "dir") == 0){
		fp = fopen("dir.txt", "r");
		while(fgets(buffer, n, fp) != NULL)
			printf("%s", buffer);
        } else if(strcmp(input.argument[1], "environ") == 0){
		puts("Prints the environment strings.");
        } else if(strcmp(input.argument[1], "echo") == 0){
		puts("Writes arguments to standard output.");
        } else if(strcmp(input.argument[1], "pause") == 0){
		puts("Halts shell activity until enter key is pressed.");
        } else if(strcmp(input.argument[1], "help") == 0){
		fp = fopen("help.txt", "r");
		while(fgets(buffer, n, fp) != NULL)
			printf("%s", buffer);
	} else puts("Not a built in command.");
}
