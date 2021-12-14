#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>      
#include <sys/wait.h>

#include "fs.h"

#define PROMPT "(|): "
#define DELIMITERS " \t\r\n\v\f"
#define MAX_LINE 1024
#define MAX_TOKEN 128

void loop();
char *read_command();
char **parse(char*);
int execute(char**);
void sig_handler(int);
int parse_special_cmds(char **);
int ls_exe(int , char* , char* );

int main(int argc, char *argv[]){
	printf("vim\n");
	for(int i=0; i<= _NSIG; i++){
		signal(i, sig_handler);
	}
	loop();
	return 0;
}

void sig_handler(int sig){
	if(0){
		if(sig == SIGTERM){
		exit(1);
		}

		else{
		/* Not sure if we have to handle all signals a certain way, but
		 * for now any signal will not affect the shell and thus it will
		 * not crash from a signal*/
		}
	}
}


void loop(){
	char *command;
	char **tokens;
	int status = 0;

	while (!status) {
		/* print prompt */
		write(STDOUT_FILENO, PROMPT ,6);
		/* tiny sleep to help printing delays */
		sleep(0.01);

		/* read and handle command */
		command = read_command();
		tokens = parse(command); 
		status = execute(tokens); // returns 0 on success
		free(command);
		free(tokens);
	}
}

/* reads one line from stdin */
char *read_command(){
	/* string holding the entire line */
	char *buf = malloc(sizeof(char) * MAX_LINE + 1);
	/* holds tokens */
	char c;
	
	if(!buf){
		write(STDOUT_FILENO, "malloc error for buffer\n", MAX_TOKEN);
		exit(1);
	}

	int i = 0;
	while (1){
		int sz = read(STDIN_FILENO, &c, 1);
		/* sz>0 means we read something */
		if (sz>0){
			if (c == '\n'){
				/* stop reading and attach \0 if enter is pressed */
				buf[i] = '\0';
				return buf;
			}
			else{
				buf[i] = c;
			}
			i++;
		}	
		else{ 
			// handles EOF (Ctrl + D)
			exit(1);
		}
	
	}

}

/*Parses a command into tokens based on delimiters*/
char **parse(char* command){
	char* token;
	char** tbuf= malloc((MAX_TOKEN) * sizeof(char*));

	if(!tbuf){
		write(STDOUT_FILENO, "malloc error for tokens buffer\n",
				MAX_TOKEN); exit(1);
	}

	int i = 0;
	token = strtok(command, DELIMITERS);
	while(token != NULL){
		tbuf[i] = token;
		i++;
		token = strtok(NULL, DELIMITERS);
	}
	// To make it null-terminating
	tbuf[i] = NULL;
	return tbuf;

}




int execute(char **tokens){
	/* returns 0 on success */

	/* if enter is pressed */
	if (tokens[0] == NULL){
		return 0;
	}
	/* handles exit */
	else if (strcmp(tokens[0], "exit") == 0){
		return 1;	
	}
	else if (parse_special_cmds(tokens) == 0) {
		return 0;
	}
	else{
		int status;
		pid_t pid, wpid;

		pid = fork();
		if (pid == 0){
			/* child code */
			if (execvp(tokens[0], tokens) == -1) {
				/* if exec doesn't succeed */
				printf("Error executing command!\n");
			}
			exit(1);
		}	
		else if (pid > 0){
			/* parent code */
			/* parent waits for child to finish */
			do {
				wpid = waitpid(pid, &status, 0);	
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		else{
			printf("Error Forking!\n");
		}
		return 0;
	}
	
}

int parse_special_cmds(char **tokens){
	/* returns 0 on success */
	
	/* parses ls chmod cat ... */


	if (strcmp(tokens[0], "cat") == 0){
		
		char * filename = tokens[1];
		int fd;
		void* buffer = malloc(4);

		fd = f_open(filename);
		for (int i=0;i<5;i++){
			f_read(fd, 4, buffer);
			printf("%d==%s\n", fd, (char*)buffer);
		}


	} else if (strcmp(tokens[0], "ls") == 0){
		/* call ls here */
		if(strcmp(tokens[1], "-l")==0 || 
				strcmp(tokens[1], "-F") == 0 ){
			ls_exe(1, tokens[1], tokens[2]);
		} else {
			ls_exe(0, NULL, tokens[1]);
		}
	}

	return 0;
}







int ls_exe(int isflag, char* flag, char* folder){
	/* executes ls  */
	int fd;
	if(isflag == 1){
		/* flags are supplied */
		if(flag == "-l"){

		} else if (flag == "-F"){

		}  else {

		}
	} else {
		/* no flags are supplied */
		fd = f_open(folder);
		dentry* temp = f_readdir(fd);
		while(temp != NULL){
			printf("%d\n", temp->n);
			printf("%d\n", temp->type);
			temp = f_readdir(fd); //update the temp until we reach NULL
		}
	}	
	return 0;
}



