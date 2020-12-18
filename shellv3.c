/*
 * Programmer: Valerie Lam
 * Class: CIS 3207 - Section 001
 * Due Date: October 17th, 2020
 * Assignment: Project 2 - Devloping a Linux Shell Type Program
 * Version: 3
 * Description: Create a Linux Shell - The goal for the .
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
void execute(struct stringOrganizer input);
void cd(char *array[10]);
void clear(void);
void quit(void);
void dir(char *array[10]);
void environmentStrings(void);
void echoCopy(char *array[10]);
void enterPause(void);
void help(char *array[10]);
void isPipe(struct stringOrganizer input, int pipeLocation);

//Piping not yet implemented.
//Background execution not yet implemented.

int main(int argc, char *argv[]){
	int i, batch_validity = -1, fileFlag = -1, pipeFlag = -1, pipeLocation = -1, bkrdFlag = -1;
	char *buffer = NULL;
	char *token = (char*) malloc(100);
	FILE *stream = NULL;

	//Memory allocation of the two input pointer variables.
	struct stringOrganizer input;
	input.command = (char*) malloc(20);

	for (i = 0; i < 10; i++){
		input.argument[i] = malloc(10 * sizeof(char*));
	}
/*
	for (i = 0; i < 10; i++){
                input.argument[i] = NULL;
        }
*/
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
//			puts("Before storing command");
			strcpy(input.command, token);
//			puts("Stores command");

			token = strtok(NULL, " ");
//			printf("2: %s\n", token);
			while (token != NULL){
//				printf("The value of i in tokenizing loop is %d\n", i);
//				printf("The value of arg %d token is %s\n", i, token);
				strcpy(input.argument[i++], token); //Seg fault with this line.
//				printf("Get the token within loop\n");
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

			for(i = 0; i < 10; i++){
				if(strcmp(input.argument[i], "|") == 0){
					pipeFlag = 1;
					pipeLocation = i;
					break;
				} else pipeFlag = 0;
			}


			for(i = 0; i < 10; i++){
                                if(strcmp(input.argument[i], "&") == 0){
                                        bkrdFlag = i;
                                        break;
                                } else bkrdFlag = 0;
                        }

//			printf("The final value of fileFlag is: %d\n", fileFlag);

			//Save a copy of the original stdin and stdout streams.
			int stdin_copy = dup(STDIN_FILENO);
                       	int stdout_copy = dup(STDOUT_FILENO);

//			int pipefds[2];
//			isPipe(input, pipefds);

			for(i = 0; i < 10; i++){
				if(input.argument[i] == NULL)
					printf("null.");
				else
					printf("%s.", input.argument[i]);
			}
			puts("");

			//Note: pointer space not allocated.
			int *fp;
			if (fileFlag > 0){
				fp = inputOutputFiles(input);
			}

			if(pipeFlag > 0){
				isPipe(input, pipeLocation);
			} else {
			//Call the function execute while passing in the structure containing the command and args.
				execute(input);
			}

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

//Is there a pipe?
//If there is, identify the commands on either side of the pipe.
//Once identified, is it built in or not?
//Run matching code.

void isPipe(struct stringOrganizer input, int pipeLocation){
	puts("In piping function");
	pid_t childPIDOne, childPIDTwo;
	int builtOne = -1, builtTwo = -1, i, x;

	char *firstCommand[10];
	char *secondCommand[10];
	for (i = 0; i < 10; i++){
                firstCommand[i] = malloc(10 * sizeof(char*));
		secondCommand[i] = malloc(10 * sizeof(char*));
        }

//	puts("Created and allocated space for first and second command arrays");
/*
	for(i = 0; i < 10; i++){
		firstCommand[i] == NULL;
		secondCommand[i] == NULL;
	}
*/
	strcpy(firstCommand[0], input.command);
	i = 0;
	while(i < pipeLocation){
		//Note: the firstCommand[0] holds the first command, and 1 onward contain firstCommand arguments.
		strcpy(firstCommand[i+1], input.argument[i]);
		i++;
	}
	i++; //This addition is meant to increment the location index past the pipe.
	x = 0;
	while(i < 10){
		strcpy(secondCommand[x], input.argument[i]);
		x++;
		i++;
	}

	puts("Finished splitting commands");

	puts("\nFinished first command:");
	for(i = 0; i < 10; i++){
		printf("%s.", firstCommand[i]);
	}
	puts("");

	puts("\nFinished second command:");
	for(i = 0; i < 10; i++){
		printf("%s.", secondCommand[i]);
	}
	puts("");
/*
	if(strcmp(input.command, "cd") == 0){
		builtOne = CD;
        } else if(strcmp(input.command, "clr") == 0){
		builtOne = CLR;
        } else if(strcasecmp(input.command, "quit") == 0){
		builtOne = QUIT;
        } else if(strcmp(input.command, "dir") == 0){
		builtOne = DIRECT;
        } else if(strcmp(input.command, "environ") == 0){
		builtOne = ENVIRON;
        } else if(strcmp(input.command, "echo") == 0){
		builtOne = ECHO;
        } else if(strcmp(input.command, "pause") == 0){
		builtOne = PAUSE;
        } else if(strcmp(input.command, "help") == 0){
		builtOne = HELP;
	} else builtOne = 0;

	puts("Identified matching builtOne command");
*/
//	pipeLocation++;
/*
	if(strcmp(input.argument[pipeLocation], "cd") == 0){
                builtTwo = CD;
        } else if(strcmp(input.argument[pipeLocation], "clr") == 0){
                builtTwo = CLR;
        } else if(strcasecmp(input.argument[pipeLocation], "quit") == 0){
                builtTwo = QUIT;
        } else if(strcmp(input.argument[pipeLocation], "dir") == 0){
                builtTwo = DIRECT;
        } else if(strcmp(input.argument[pipeLocation], "environ") == 0){
                builtTwo = ENVIRON;
        } else if(strcmp(input.argument[pipeLocation], "echo") == 0){
                builtTwo = ECHO;
        } else if(strcmp(input.argument[pipeLocation], "pause") == 0){
                builtTwo = PAUSE;
        } else if(strcmp(input.argument[pipeLocation], "help") == 0){
                builtTwo = HELP;
        } else builtTwo = 0;
*/
//	puts("Identified matching builtTwo command");
	int pfds[2], status = 0;
	if (pipe(pfds) == 0){
		if ((childPIDOne = fork()) == 0){
		//if(childPIDOne == 0){
			puts("Inside first fork");
//			printf("The value of builtOne is %d.\n", builtOne);
//			close(1); //Closes standard output. Nothing will be printed to screen after this.
			dup2(pfds[1], 1);
			close(pfds[0]);
			close(pfds[1]);
/*			switch(builtOne){
			case CD:
				cd(firstCommand);
				break;
			case CLR:
				clear();
				break;
			case QUIT:
				quit();
				break;
			case DIRECT:
				dir(firstCommand);
				break;
			case ENVIRON:
				environmentStrings();
				break;
			case ECHO:
				echoCopy(firstCommand);
				break;
			case PAUSE:
				enterPause();
				break;
			case HELP:
				help(firstCommand);
				break;
			default:
*/	                        i = 0;
				while(i < 10){
                                	if(strcmp(firstCommand[i], "") == 0){
                                        	//Insert a null pointer.
                                        	firstCommand[i] = NULL;
					}
					i++;
                                }

				for(i = 0; i < 10; i++){
                                        if(firstCommand[i] == NULL)
                                                printf("null.");
                                        else
                                                printf("%s.", firstCommand[i]);
                                }

				if(execvp(firstCommand[0], firstCommand) == -1){
        	                        puts("Execvp failed.");
//					printf(stderr, "execvp() failed. errno = %d", errno);
	                        	exit(1);
				}
//				break;
//			}
			exit(0);
		} else {
			if((childPIDTwo = fork()) == 0){
			//if(childPIDTwo == 0){
				puts("Inside second fork");
//				close(0);

				puts("Before dup2");
				dup2(pfds[0], 0);
				puts("After dup2");
				close(pfds[1]);
				close(pfds[0]);

//				puts("Before switch");
//				printf("The value of builtTwo is %d.\n", builtTwo);
/*				switch(builtTwo){
                 		        case CD:
                                		cd(secondCommand);
                                		break;
                        		case CLR:
                                		clear();
                                		break;
                        		case QUIT:
                                		quit();
                                		break;
                        		case DIRECT:
                                		dir(secondCommand);
                                		break;
					case ENVIRON:
                                		environmentStrings();
                                		break;
                        		case ECHO:
                                		echoCopy(secondCommand);
                                		break;
                        		case PAUSE:
                                		enterPause();
                                		break;
                        		case HELP:
                                		help(secondCommand);
                                		break;
                        		default:
*/						i = 0;
						while(i < 10){
//							puts("\nInside second fork while loop");
							if(strcmp(secondCommand[i], "") == 0){
								secondCommand[i] = NULL;
//								printf("Set equal to null!\n");
							}
						i++;
						}

						puts("PRINTING ARGS:");
						for(i = 0; i < 10; i++){
							if(secondCommand[i] == NULL)
								printf("null.");
							else
								printf("%s.", secondCommand[i]);
						} puts("");

                                		if(execvp(secondCommand[0], secondCommand) == -1){
                                        		puts("Execvp failed.");
//							printf(stderr, "execvp() failed. errno = %d", errno);
                                        		exit(1);
                                		}
//						break;
		//		}
				exit(0);
			} else {
				puts("Waiting on child process...");
				int statusTwo;

				close(pfds[0]);
				close(pfds[1]);

				waitpid(childPIDOne, &status, 0);
				waitpid(childPIDTwo, &statusTwo, 0);
				//printf("The value of wait is %d.\n", num);
				puts("Finished waiting for child process");
//				exit(0);
			}
		}
	}
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

void execute(struct stringOrganizer input){
	puts("In execute function");
	pid_t PID;
	int status = 0;
	int i, whatisEx = 999;

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




	//Code for piping.
/*
	} else if(pipeFlag == 1){
		if(pipe(pfds) == 0){
			if(fork() == 0){
				close(1); //Close stdout
				dup2(pfds[1], 1); //Wire stdout to pipe's write end
				close(pfds[0]); //Close read end of the pipe
				//The purpose of this bit of code is to move shift all strings in the array to the right by one.
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
				if(execvp(input.command, input.argument) == -1){
                                	puts("Execvp failed.");
                        		exit(0);
				}
			} else if (fork() == 0){
				close(0);
				dup2(pfds[0], 0);
				if(execvp(input.command, input.argument) == -1){
                                        puts("Execvp failed.");
                                        exit(0);
                                }
			} else if (fork() > 0){
				int num = wait(&status);
        	                printf("Value returned from wait: %d\n", num);
	                        printf("Finished waiting for child process to terminate.\n");
			}
		}
*/





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

void cd(char *array[10]){
	puts("In cd function");
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

void clear(){
	printf("\033[H\033[2J");
}

void quit(){
	puts("Exiting shell.");
	exit(0);
}

void dir(char *array[10]){
	puts("In dir function");
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

void environmentStrings(){
	puts("In environ function");
	printf("PATH: %s\n", getenv("PATH"));
	printf("HOME: %s\n", getenv("HOME"));
	printf("ROOT: %s\n", getenv("ROOT"));
}

void echoCopy(char *array[10]){
	int i = 0;
	const char null[5] = {'\0'};
	while(strcmp(array[i], null) != 0){
		printf("%s ", array[i++]);
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

//CHANGE HELP.TXT TO README.MD.
//SHOULD THE ARRAY ELEMENT BE 0 OR 1?
void help(char *array[10]){
	int n = 512;
	char buffer[n];
	FILE *fp;
	if(strcmp(array[0], "cd") == 0){
		puts("Changes the shell working directory.");
        } else if(strcmp(array[0], "clr") == 0){
		puts("Clears the screen.");
        } else if(strcasecmp(array[0], "quit") == 0){
		puts("Quits the shell program.");
        } else if(strcmp(array[0], "dir") == 0){
		fp = fopen("dir.txt", "r");
		while(fgets(buffer, n, fp) != NULL)
			printf("%s", buffer);
        } else if(strcmp(array[0], "environ") == 0){
		puts("Prints the environment strings.");
        } else if(strcmp(array[0], "echo") == 0){
		puts("Writes arguments to standard output.");
        } else if(strcmp(array[0], "pause") == 0){
		puts("Halts shell activity until enter key is pressed.");
        } else if(strcmp(array[0], "help") == 0){
		fp = fopen("help.txt", "r");
		while(fgets(buffer, n, fp) != NULL)
			printf("%s", buffer);
	} else puts("Not a built in command.");
}
