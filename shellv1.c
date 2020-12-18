/*
 * Programmer: Valerie Lam
 * Class: CIS 3207 - Section 001
 * Due Date: October 9th, 2020
 * Assignment: Project 2 - Devloping a Linux Shell Type Program
 * Version: 1
 * Description: Create a Linux Shell -- The goal for the first submission is built-in commands and file redirection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void execute(char command[20], char argument[10][10]);
int isItBuiltIn(char command[20]);
void cd(char argument[10][10]);
void clear(void);
//
//QUESTIONS FOR MR. FU:
//1. Need help figuring out fork.
//2. How do I copy the tokenized command line words into the object? Can I pass them to other functions that way?
//   Can I refer to a passed object variable if its renamed in a passed function?
//3. How do I implement commands that are supposed to be built in? (The big 8.)
//4. In general, how does file redirection work?
//5. What is wrong with the string I am trying to print? Am I doing something wrong with memory?

int main(){
	int i;
	char buffer[100];
	char *token = (char*) malloc(100);
	char command[20];
	char argList[10][10];

	for(i = 0; i < 100; i++){
		buffer[i] = '\0';
	}
	while(1){
		printf("myshell> ");
		fgets(buffer, 100, stdin);

		//Remove the trailing newline character.
		for(i = 0; i < 100; i++){
			if(buffer[i] == '\n')
				buffer[i] = '\0';
		}

		token = strtok(buffer," ");
		printf("breakline is not first token\n");
		if(strcmp(token, "exit") == 0)
			break;
		else{
			i = 0;
			strcpy(command, token);
			printf("%s\n", command);
			printf("before while loop\n");
			while (token != NULL){
				strcpy(argList[i], token);
				token = strtok(NULL, " ");
			}
			execute(command, argList);
		}


	}
	return EXIT_SUCCESS;
}

void execute(char command[20], char argument[10][10]){
	pid_t childPID;
	int status;
	int check = isItBuiltIn(command);
	int i;
/*	FILE *input, *output;

	for(i = 0; i < 10; i++){
		if(strcmp(argument[i], ">") == 0){
			*input = argument[++i];
			fopen(input, "r");
		}else if(strcmp(argument[i], "<") == 0){
			*output = argument[++i];
		{
	}

	fopen(input, "r");
*/

	if(strcmp(command, "cd") == 0){
		cd(argument);
	}

	if(strcmp(command, "clr") == 0){
		clear();
	}

	if (check == 0){
		childPID = fork();
		if(childPID < 0) {
			printf("Forking child process unsuccessful.\n");
			exit(1);
		} else if(childPID == 0){
			execvp(command, argument[0]);
			while(wait(&status) != childPID){
				free(command);
			}
		}
	}
}
/*
int isItBuiltIn(char *command){
	if(strcmp(command, "cd") == 0)
		return 1;
	else if(strcmp(command, "clr") == 0)
		return 1;
	else if(strcmp(command, "dir") == 0)
		return 1;
	else if(strcmp(command, "environ") == 0)
		return 1;
	else if(strcmp(command, "echo") == 0)
		return 1;
	else if(strcmp(command, "help") == 0)
		return 1;
	else if(strcmp(command, "pause") == 0)
		return 1;
	else if(strcmp(command, "quit") == 0)
		return 1;
	else return 0;
}

*/

void cd(char argument[10][10]){
        char *cwd;
        char *slash = "/";
        int didItWork;
                if(argList[0] != NULL){
                        strcat(slash, argList[0])
                        if(getcwd(cwd, sizeof(cwd)) != NULL){
                                int didItWork = chdir(slash);
                                if(didItWork != 0){
                                        puts("Error: chdir()");
                                }
                } else {
                        if(getcwd(cwd, sizeof(cwd)) != NULL){
                                chdir("..");
                                if(didItWork != 0){
                                        puts("Error: chdir()");
                                }
                }
}

void clear(){
	printf("\033[2J");
}
