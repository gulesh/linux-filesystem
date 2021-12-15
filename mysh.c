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
#define GREEN   "\x1b[32m"
#define RESET   "\x1b[0m"

#define MAXMORENEWLINE 3

void loop();
char *read_command();
char **parse(char*);
int execute(char**);
void sig_handler(int);
int parse_special_cmds(char **);
int ls_exe(int , char* , char* );

int pwd_fd = 0; 

int main(int argc, char *argv[]){
	for(int i=0; i<= _NSIG; i++){
		signal(i, sig_handler);
	}
	init_library("DISK");
	loop();
	return 0;
}

void sig_handler(int sig){
	/*change to 1 for easy exit*/
	if(1){
		if(sig == SIGINT){
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
	/*When parse special returns -1 (normal fail), it still goes to else
	statement. Reason for mkdir happening twice*/
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
	/* returns 0 if a special token is found */
	/* 1 if not */
	
	/* parses ls chmod cat ... */


	if (strcmp(tokens[0], "cat") == 0){
		/* cat is here */
		
		char * filename = tokens[1];
		int fd, numread;
		void* buffer = malloc(4);

		fd = f_open(filename);
		while (1){
			numread = f_read(fd, 1, buffer);
			if (numread == 0){ 
				break;
			}

			/* replace carriage returns with \n */
			if (*(char*)buffer == '\r') *(char*)buffer='\n';

			printf("%s", (char*)buffer);
		}
		printf("\n");
		return 0;

	} else if (strcmp(tokens[0], "more") == 0) {
		/* more is here */

		/* TODO make space or enter print more TODO */

		char * filename = tokens[1];
		int fd, numread, newlinecount;
		void* buffer = malloc(4);

		fd = f_open(filename);
		while (1){
			numread = f_read(fd, 1, buffer);
			if (numread == 0){ 
				break;
			}

			/* replace carriage returns with \n */
			if (*(char*)buffer == '\r') *(char*)buffer='\n';

			/* cutoff after a certain number of newlines */
			if (*(char*)buffer == '\n') newlinecount ++;
			if (newlinecount >= MAXMORENEWLINE-1) break;

			printf("%s", (char*)buffer);
		}
		printf("\n");
		return 0;



	} 
	else if (strcmp(tokens[0], "ls") == 0){
		/* ls here */
		if (tokens[1] == NULL){
			ls_exe(0,NULL,NULL);
		}

		else if(strcmp(tokens[1], "-l")==0 || 
				strcmp(tokens[1], "-F") == 0 ){
			ls_exe(1, tokens[1], tokens[2]);
		} 
		else{
			ls_exe(0, NULL, tokens[1]);
		}
		return 0;



	} else if (strcmp(tokens[0], "pwd") == 0){
		/* pwd is here */
		
		printf("%s\n", pwd);
		return 0;



	} else if (strcmp(tokens[0], "cd") == 0){
		/* cd is here */

		/* store pwd in case invalid path is supplied */
		char* temp = malloc(MAX_LEN*MAX_LEN);	
		strcpy(temp, pwd);

		if (!tokens[1]){
			/* change to root directory if no path specified */
			strcpy(temp, "/");
			pwd_fd = 0;
		} 
		else if (!strcmp(tokens[1],".")) {
			/* stay in the same directory */
		} 
		else {
			if (tokens[1][0] == '/'){
				/* absolute path specified */
				strcpy(temp, tokens[1]);
				int fd = f_opendir(temp);
				if (fd == -1){
					/* invalid path */
					return 0;
				}
			}
			else {
				/* relative path specified */
				strcat(temp, tokens[1]);
				int status = f_opendir(temp);
				if (status == -1){
					/* invalid path */
					printf("invalid\n");
					return 0;	
				}
				pwd_fd = status;
			}
		memcpy(temp + strlen(temp), "/", 1);
		memcpy(pwd,temp, strlen(temp));
		printf("Temp: %s\n", temp);
		printf("PWD: %s\n", pwd);

		}
		
		return 0;
	} 
	else if (strcmp(tokens[0], "mkdirr") == 0){
		f_mkdir(tokens[1]);
		return 0;
	}


	/* no special commands parsed */
	return 1;

}



int ls_exe(int isflag, char* flag, char* folder){
	/* executes ls  */
	int fd = pwd_fd;
	if(isflag == 1){
		/* flags are supplied */
		if(flag == "-l"){

		} else if (flag == "-F"){

		}  else {

		}
	} else {
		if (folder != NULL){
			fd = f_opendir(folder);
			pwd_fd = fd;
		}
		/* no flags are supplied */
		dentry* temp = malloc(sizeof(dentry));
		temp = f_readdir(pwd_fd);
		int count = 1;
		while(temp != NULL){
			char *name = malloc(temp->length);
			name =(char *)((void*)temp+sizeof(dentry));
			if (temp->type == DIRECTORY || temp->type == MNT_PNT)
				printf("%s%s%s\t",GREEN, name, RESET);
			else
				printf("%s\t",name);
			if (count %4 == 0)
				printf("\n");
			count+=1;	
			temp = f_readdir(fd); //update the temp until we reach NULL
		}
		if ((count-1) % 4 != 0)
			printf("\n");
	}	
	return 0;
}

