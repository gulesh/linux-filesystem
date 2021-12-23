#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>      
#include <sys/wait.h>
#include <util.h>

#include "fs.h"

#define PROMPT "(|): "
#define DELIMITERS " \t\r\n\v\f"
#define MAX_LINE 1024
#define MAX_TOKEN 128
#define GREEN   "\x1b[32m"
#define RESET   "\x1b[0m"

#define MAXMORENEWLINE 5

void loop();
char *read_command();
char **parse(char*);
int execute(char**);
void sig_handler(int);
int parse_special_cmds(char **);
int ls_exe(int , char* , char* );
void absolute_path(char *, char *);
int is_in_special(char*);
void redirect();

int pwd_fd = 0;  
int redirect_flag = 0;
char *output;
/*redirection file descriptor*/
int d;

int main(int argc, char *argv[]){
	for(int i=0; i<= _NSIG; i++){
		signal(i, sig_handler);
	}
	init_library("DISK");
	loop();
	//free(output);
	close_library();
	return 0;
}

void sig_handler(int sig){
	/*change to 1 for easy exit*/
	if(1){
		if(sig == SIGINT){
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
		redirect();
		free(command);
		free(tokens);
	}
}

void redirect(){
	if (redirect_flag == 1){
		FILE *f = fopen("re", "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET); 

		char *b = malloc(fsize + 1);
		fread(b, fsize, 1, f);
		fclose(f);

		b[fsize] = '\0';
		/*open new file in out file system using global 'output'*/
		char* temp = malloc(MAX_LEN*MAX_LEN);
        absolute_path(output, temp);
        char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
        strcpy(temp_for_open, temp);
		int new = f_open(temp_for_open, 1);
		f_write(b, fsize, new);
		free(temp);
		free(temp_for_open);
		free(b);
		redirect_flag = 0;
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
		if (strcmp(token, ">") == 0){
			redirect_flag = 1;
			output = malloc(MAX_LEN);
			output = strtok(NULL, DELIMITERS);
			break;
		}
		tbuf[i] = token;
		i++;
		token = strtok(NULL, DELIMITERS);
	}
	// To make it null-tecrminating
	tbuf[i] = NULL;
	return tbuf;

}

int ls_exe(int isflag, char* flag, char* folder){
	FILE* f;
	if(isflag){
		if(flag == "l"){

		} else if(flag == "F"){

		} else{
			f = f_open(folder, "r");
			dentry* temp = f_readdir();
			while(temp != NULL){
				printf("%d\n", temp->n);
				printf("%d\n", temp->n);
			}
		}
	}
	
}

int execute(char **tokens){
	/* returns 0 on success */

	/* if enter is pressed */
	if (tokens[0] == NULL){
		redirect_flag = 0;
		return 0;
	}
	/* handles exit */
	else if (strcmp(tokens[0], "exit") == 0){
		redirect_flag = 0;
		return 1;	
	}
	/* when parse special returns -1 (normal fail), it still goes to
	 * else statement. 
	 * reason for mkdir happening twice */
	else if (is_in_special(tokens[0])) {
		redirect_flag = 0;
		return parse_special_cmds(tokens);
	}
	else if (strcmp(tokens[0], "ls") == 0){
		/* call ls here */

	}
	else{
		int status;
		pid_t pid, wpid;
		pid = fork();
		if (pid == 0){
			/* child code */
			if (redirect_flag == 1){
				int fd = open("re", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				/*d is a global file descriptor for the file with stdout*/
				d = dup2(fd, 1);
				close(fd);
			}
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

int ls_exe(int isflag, char* flag, char* folder){
	FILE* f;
	if(isflag){
		if(flag == "-l"){

		} else if(flag == "-F"){

		}  else{

		}
	} else{
		f = f_open(folder, "r");
		dentry* temp = f_readdir(f);
		while(temp != NULL){
			printf("%d\n", temp->n);
			rintf("%d\n", temp->n);
			temp = f_readdir(f); //update the temp until we reach NULL
		}
		if(temp == NULL){
			return 1; //check if 0 or 1
		} else{
			return -1;
		}
	}	
}


int parse_special_cmds(char **tokens){
	/* returns 0 if a special token is found */
	/* 1 if not */
	
	/* parses ls chmod cat ... */


	if (strcmp(tokens[0], "cat") == 0){
		/* cat is here */
		
		int fd, numread;
		void* buffer = malloc(2);

		char* temp = malloc(MAX_LEN*MAX_LEN);	
		absolute_path(tokens[1], temp);
		char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
		strcpy(temp_for_open, temp);

		if (!(fd = f_open(temp_for_open, 0))){
			/* error opening file */
			printf("cat: error opening file\n");
		}
		while (1){
			numread = f_read(fd, 1, buffer);
			if (numread == 0){ 
				break;
			}

			/* replace carriage returns with \n */
			if (*(char*)buffer == '\r') *(char*)buffer='\n';

			printf("%c", *(char*)buffer);
		}
		printf("\n");

		free(temp);
		free(temp_for_open);
		free(buffer);
		return 0;

	} else if (strcmp(tokens[0], "more") == 0) {
		/* more is here */

		/* TODO make space or enter print more TODO */

		int newlinecount;
		int fd, numread;
		void* buffer = malloc(2);

		char* temp = malloc(MAX_LEN*MAX_LEN);	
		absolute_path(tokens[1], temp);
		char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
		strcpy(temp_for_open, temp);

		fd = f_open(temp_for_open,0);


		char *line = NULL;
		size_t len = 0;
		ssize_t read;


		do {
			newlinecount =0;
			while (1){
				numread = f_read(fd, 1, buffer);
				if (numread == 0){ 
					printf("\n");
					free(buffer);
					free(temp);
					free(temp_for_open);
					return 0;
				}

				/* replace carriage returns with \n */
				if (*(char*)buffer == '\r') *(char*)buffer='\n';

				/* cutoff after a certain number of newlines */
				if (*(char*)buffer == '\n') newlinecount ++;
				if (newlinecount >= MAXMORENEWLINE) break;

				printf("%c", *(char*)buffer);
			}

		} while ((read = getline(&line, &len, stdin)) != -1);


//		while (1){
//			numread = f_read(fd, 1, buffer);
//			if (numread == 0){ 
//				break;
//			}
//
//			/* replace carriage returns with \n */
//			if (*(char*)buffer == '\r') *(char*)buffer='\n';
//
//			/* cutoff after a certain number of newlines */
//			if (*(char*)buffer == '\n') newlinecount ++;
//			if (newlinecount >= MAXMORENEWLINE-1) break;
//
//			printf("%c", *(char*)buffer);
//		}
		printf("\n");
		free(buffer);
		free(temp);
		free(temp_for_open);
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



//	} else if (strcmp(tokens[0], "rm") == 0){
//		/* rm is parsed here */
//		char* temp = malloc(MAX_LEN*MAX_LEN);	
//		if (tokens[1][0] == '/'){
//				/* absolute path specified */
//				strcpy(temp, tokens[1]);
//				strcat(temp, "/");
//				char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
//				strcpy(temp_for_open, temp);
//				int fd = f_rm(temp_for_open);
//				if (fd == -1){
//					/* invalid path */
//					return 0;
//				}
//			}
//			else {
//				printf("In relative\n");
//				/* relative path specified */
//				strcat(temp, tokens[1]);
//				strcat(temp, "/");
//				printf("len of temp: %ld\n", strlen(temp));
//				printf("temp: %s\n", temp);
//				char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
//				strcpy(temp_for_open, temp);
//				int status = f_rm(temp_for_open);
//				if (status == -1){
//					/* invalid path */
//					printf("invalid\n");
//					return 0;	
//				}
//			}
//		return 0;	
//
	} else if (strcmp(tokens[0], "rm") == 0){ } else if
		(strcmp(tokens[0], "rmdir") == 0 || strcmp(tokens[0], "rm") ==
		 0){
		char* temp = malloc(MAX_LEN*MAX_LEN);	
		char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
		absolute_path(tokens[1], temp);
		strcpy(temp_for_open, temp);
		int status = f_rmdir(temp_for_open);
		if (status == -1){
			/* invalid path */
			return 0;	
		}
		free(temp_for_open);
		free(temp);
		return 0;	

	} else if (strcmp(tokens[0], "pwd") == 0){
		/* pwd is here */
		
		printf("%s\n", pwd);
		return 0;

	} else if (strcmp(tokens[0], "cd") == 0) {
		/* cd is here */
		/* store pwd in case invalid path is supplied */
		char* temp = malloc(MAX_LEN*MAX_LEN);	

		if (!tokens[1]){
			/* change to root directory if no path specified */
			strcpy(temp, "/");
			strcpy(pwd, temp);
			pwd_fd = 0;
		} 
		else if (!strcmp(tokens[1],".")) {
			/* stay in the same directory */
		} 
		else {
			/* parse path to dir and open */
			absolute_path(tokens[1], temp);
			char *temp_for_open = malloc(MAX_LEN*MAX_LEN);
			strcpy(temp_for_open, temp);
			int status = f_opendir(temp_for_open);
			if (status == -1){
				/* invalid path */
				return 0;	
			}
			pwd_fd = status;
			strcpy(pwd,temp);
			free(temp_for_open);
		}
		free(temp);
		
		return 0;

	} else if (strcmp(tokens[0], "mkdir") == 0){
		f_mkdir(tokens[1]);
		return 0;
	}


/* no special commands parsed */
return 1;

}

int is_in_special(char *cmd){
	int n = 8;
	char special[n][MAX_LEN];

	strcpy(special[0] , "cd");
	strcpy(special[1] , "ls");
	strcpy(special[2] , "mkdir");
	strcpy(special[3] , "pwd");
	strcpy(special[4] , "rm");
	strcpy(special[5] , "cat");
	strcpy(special[6] , "rmdir");
	strcpy(special[7] , "more");

	for (int i = 0; i<n; i++){
		if(strcmp(cmd, special[i]) == 0){
			return 1;
		}
	}
		fd = f_open(filename);
		f_read(fd, 20, buffer);
		printf("%d==%s\n", fd, (char*)buffer);
	} else if (strcmp(tokens[0], "ls") == 0){
		/* call ls here */
		if(strcmp(token[1], "-l") || strcmp(token[1], "-F")){
			ls_exe(1, token[1], token[2]);
		} else{
			ls_exe(0, NULL, token[1]);
		}
	}

	return 0;
}


void absolute_path(char * path, char* temp){
/* parse string into absolute path */
	if (path[0] == '/'){
		/* absolute path specified */
		strcpy(temp, path);
		if (path[1] != '\0'){
			/* account for just "/" path */
			strcat(temp, "/");
		}
	} else {
		/* relative path specified */
		strcpy(temp, pwd);
		strcat(temp, path);
		strcat(temp, "/");
	}
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
			if (count %4 == 0) printf("\n");
			count+=1;	
			temp = f_readdir(fd); //update the temp until we reach NULL
		}
		if ((count-1) % 4 != 0) printf("\n");
		free(temp);
	}
	return 0;
}

