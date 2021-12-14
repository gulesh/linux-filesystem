#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>

#include "linkedlist.h"
#include "job.h"

#define PROMPT "(|): "
#define DELIMITERS " \t\r\n\v\f"
#define SEMICOLON ";"
/* define how many characters are accepted in each type */
#define MAX_LINE 1024
#define MAX_COMMAND 100
#define MAX_TOKEN 128
#define MAX_TKN_SZ 128
#define MAX_COLON 10

/* Maximum number of signals (to block) defined */
#ifdef SIGRTMAX
#define MAXSIG SIGRTMAX
#endif
#ifndef MAXSIG
#define MAXSIG 64
#endif

/* globar variables */
int jobidcounter = 1; /* assigns job id to a job */
int eof = 0; /* catches eof */

void loop();
char *read_command();
char **semiparser(char*);
char **parse(char*);
int createjob(char**);
int execute(struct Node* );
void sig_handler(int);
static void sigactionhandler(int, siginfo_t*, void*);
void siddefault();
int fg(struct Node*);
void bg(struct Node*);
void killpid(struct Node*, int);
void free2d(char**, int);
int getlen(char** );

struct Node* head = NULL;

struct termios term1;

void handler(int);

static void sigactionhandler(int sig, siginfo_t *si, void *unused){
	/* register a handler for CHLD signals */
	struct Node *childnode = findnodebypid(si->si_pid);
	//stopped
	/* restore terminal termios */
	if (si->si_code == CLD_STOPPED){
		if (tcgetattr(STDIN_FILENO, &(childnode->data->term)) != 0)
                        perror("tcgetattr() error\n");
		childnode->data->running = 0;
		if (tcsetpgrp(STDOUT_FILENO, getpgrp()) != 0)
        		perror("tcsetpgrp() error in sigaction (stopped)");
		if (tcsetattr(STDIN_FILENO, TCSANOW, &term1) != 0)
      			perror("tcsetattr() error");
	}
	//continued
	else if (si->si_code == CLD_CONTINUED){
		childnode->data->running = 1;
	}
	//killed
	/* remove node
	 * reset termios*/
	else if (si->si_code == CLD_KILLED){
		removenode(childnode);
		if (tcsetpgrp(STDOUT_FILENO, getpgrp()) != 0)
        		perror("tcsetpgrp() error in sigaction (killed)");
		if (tcsetattr(STDIN_FILENO, TCSANOW, &term1) != 0)
      			perror("tcsetattr() error");
	}
	//exited
	else if (si->si_code == CLD_EXITED){
		removenode(childnode);
		if (tcsetpgrp(STDOUT_FILENO, getpgrp()) != 0)
        		perror("tcsetpgrp() error in sigaction (exited)");
		if (tcsetattr(STDIN_FILENO, TCSANOW, &term1) != 0)
      			perror("tcsetattr() error");
	}
	else{
	}
}

int main(int argc, char *argv[]){
	/* default all signals to the voiding handler */
	for(int i=0; i<= MAXSIG; i++){
		signal(i, SIG_IGN);
	}
	/* register a handler for sigchild */
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sigactionhandler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("failed to register sigaction handler\n");
	}
	/* enter parse-execute loop */
    	loop();
	freell(head);
	free(head);
    	return 0;
}

/* default signal handler */
void sigdefault(){
	for(int i=0; i<= MAXSIG; i++){
        signal(i, SIG_DFL);
    }
}


/* basic loop is adapted from
 * https://brennan.io/2015/01/16/write-a-shell-in-c/
 */

/* main execution loop */
void loop(){
	char *command;
	char **tokens;
	char **commands;
	int status = 1;

	while (status) {
		/* write prompt */
		write(STDOUT_FILENO,PROMPT,5);
		/* read command */
		command = read_command();
		if(eof == 1){
			/* handle end of file */
			free(command);
			exit(1);
		}
		if(command == NULL){
			free(command);
			exit(1);
		}
		/* parse commands by semicolons */
		commands = semiparser(command);
		free (command);
		int i = 0;
		while (commands[i] != NULL){
			/* for each command delimeterd by semicolon: */
			/* parse into tokens */
			tokens = parse(commands[i]);
			/* create job with tockens as arguments */
			int state = createjob(tokens);
			if (state == 1)
			/* execute the last added job */	
				status = execute(pop());
			if (head == NULL)
				jobidcounter = 1;

			/* freeing tokens */
			free2d(tokens, MAX_TOKEN);
			i++;
		}
		/* freeing commands and tokens*/
		free2d(commands, MAX_COLON);
    	}
}

/*Reads one line from stdin*/
char *read_command(){
	char *buf = malloc(sizeof(char) * MAX_LINE);
	char c;

	if(!buf){
		/* error allocating memory */
		printf("malloc error for buffer in read_command\n"); 
		return NULL;
	}

	int i = 0;
	while (1){
		int sz = read(STDIN_FILENO, &c, 1);
		/*sz>0 means we read something*/
		if (sz>0){
			if (c == '\n'){
				/* return was pressed, terminate str */
				buf[i] = '\0';
				return buf;
			}
			else{
				/* read character  */
				buf[i] = c;
		    }
		    i++;
		}
		else if (sz==0){
			// handles (Ctrl + D)
			eof = 1;
			free(buf);
			return NULL;
		}

	}

}

/* parse string into commands delimeted with semicolon */
char** semiparser(char* command){
	char* token;
	char** commands = malloc(MAX_COLON*sizeof(char*));
	for (int i = 0; i<MAX_COLON; i++){
		commands[i] = malloc(MAX_COMMAND*sizeof(char));
	}
	if(!commands){
                printf("malloc error for buffer in semiparser\n");
		free2d(commands, MAX_COLON);
		return NULL;
        }

        int i = 0;
        /* tokenize string into tbuf */
        token = strtok(command, SEMICOLON);
        while(token != NULL){
        	strcpy(commands[i], token);
        	i++;
        	token = strtok(NULL, SEMICOLON);
        }
        /* indicate end of token array with NULL */
        commands[i] = NULL;
        return commands;

}

/*Parses a command into tokens based on whitespace delimiters*/
char **parse(char* command){
	char* token;
	char** tbuf= malloc((MAX_TOKEN) * sizeof(char*));
	for(int i = 0; i< MAX_TOKEN; i++){
		tbuf[i]=malloc((MAX_TKN_SZ)*sizeof(char));
	}


	if(!tbuf){
		write(STDERR_FILENO, "malloc error for tokens buffer\n", MAX_TOKEN); 
	}

	int i = 0;
	/* tokenize string into tbuf */
	token = strtok(command, DELIMITERS);
	while(token != NULL){
	strcpy(tbuf[i], token);
	i++;
	token = strtok(NULL, DELIMITERS);
	}
	/* indicate end of token array with NULL */
	tbuf[i] = NULL;
	return tbuf;

}

/* created a linkd list element with a job */
int createjob(char** tokens){
	int bg = 0;
	int i = 0;

	/* iterate to check if postfixed with & */
	while(tokens[i]!=NULL){
		int j = 0;
		while(tokens[i][j]!='\0'){
			if(tokens[i][j] == '&'){
				if(tokens[i][j+1] == '\0' && tokens[i+1] == NULL){
					if(j>0){
						tokens[i][j] = '\0';
						bg=1;
						break;

					}
					else if (i>0){
						tokens[i] = NULL;
						/* if ampersand found, set bg =1  */
						bg=1;
						break;
					}
					else{
						printf("ebash: syntax error near unexpected token '&'\n");
						return 0;
					}
								}
				else{
					printf("ebash: syntax error near unexpected token '&'\n");
					return 0;
					break;
				}
			}
			j++;
		}
		i++;
	}
	/* initialize job type */
	jobtype* job = initjob();
	job->jobid = jobidcounter;
	jobidcounter++;
	job->background = bg;
	/* allocate memory to store calling token */
    	job->t_arr= malloc((MAX_TOKEN) * sizeof(char*));
		for(int j = 0; tokens[j]!=NULL; j++){
			job->t_arr[j] = malloc((MAX_TKN_SZ)*sizeof(char));
			strcpy(job->t_arr[j], tokens[j]);
		}
	job->running = 0;
	/* create a node for the linekd list and populate with JOB */
	struct Node *node = GetNewNode(job);
	sigset_t new_set, old_set;
	/* handle race condition
	 * insert the node into the linked list*/
	sigemptyset( &new_set );
    	sigaddset( &new_set, SIGCHLD );
    	sigprocmask( SIG_BLOCK, &new_set, &old_set);
	InsertAtTail(node);
	sigprocmask( SIG_SETMASK, &old_set, NULL);
	return 1; 

}



int execute(struct Node *node){
        /*If user just presses enter*/
        if (node->data->t_arr[0] == NULL){
			/* delete node */
			/* handle a race condition */
		sigset_t new_set, old_set;
        	sigemptyset( &new_set );
        	sigaddset( &new_set, SIGCHLD );
        	sigprocmask( SIG_BLOCK, &new_set, &old_set);
		removenode(node);
		sigprocmask( SIG_SETMASK, &old_set, NULL);
		jobidcounter--;
                return 1;
        }
	/*Handles exit*/
        else if (strcmp(node->data->t_arr[0], "exit") == 0){
			/* delete exit job */
			/* handle race condition for job deletion */
		sigset_t new_set, old_set;
        	sigemptyset( &new_set );
        	sigaddset( &new_set, SIGCHLD );
      	  	sigprocmask( SIG_BLOCK, &new_set, &old_set);
		removenode(node);
		sigprocmask( SIG_SETMASK, &old_set, NULL);
                return 0;
        }
	/*Handles fg*/
	else if (strcmp(node->data->t_arr[0], "fg") == 0){
		// if #job is omitted 
		if (node->data->t_arr[1]==NULL){
			if (node->prev!=NULL){
				fg(node->prev);
			}
			else{
				printf("-ebash: fg: current: no such job\n");
			}	
		}
		else{
			if (node->data->t_arr[1][0] == '%'){
			//check that there is nothing after %jobid
			//gets pid from jobid
			char* temp = &node->data->t_arr[1][1];
			int jid = atoi(temp);
			fg(findnodebyjobid(jid));
			//return control to ebash
			if (tcsetpgrp(STDIN_FILENO, getpid()) != 0)
				perror("tcsetpgrp() error in fg in exec");
			if (tcsetattr(STDIN_FILENO, TCSANOW, &term1) != 0)
				perror("tcsetattr() error");
			
			}
			else{
				//check that there  is nothing after pid
				//convert to pid
				int pid = atoi(node->data->t_arr[1]);
				fg(findnodebypid(pid));
			}
		}
		/* race condition when deleting */
		sigset_t new_set, old_set;
                sigemptyset( &new_set );
                sigaddset( &new_set, SIGCHLD );
                sigprocmask( SIG_BLOCK, &new_set, &old_set);
                removenode(node);
                sigprocmask( SIG_SETMASK, &old_set, NULL);
		return 1;
	
	}
	else if (strcmp(node->data->t_arr[0], "bg") == 0){
		if (node->data->t_arr[1]==NULL){
                        if (node->prev!=NULL){
                                bg(node->prev);
                        }
                        else{
                                printf("ebash: bg: current: no such job\n");
                        }
                }
                else{
			if (node->data->t_arr[1][0] == '%'){
				//check that there is nothing after %jobid
				//gets pid from jobid
				char* temp = &node->data->t_arr[1][1];
				int jid = atoi(temp);
				bg(findnodebyjobid(jid));

			}
			else{
				//check that there  is nothing after pid
				//convert to pid
				int pid = atoi(node->data->t_arr[1]);
				bg(findnodebypid(pid));
			}
		}
		/* race condition when deleting */
		sigset_t new_set, old_set;
                sigemptyset( &new_set );
                sigaddset( &new_set, SIGCHLD );
                sigprocmask( SIG_BLOCK, &new_set, &old_set);
                removenode(node);
                sigprocmask( SIG_SETMASK, &old_set, NULL);
                return 1;
	}
	/* handle kill command */
	else if (strcmp(node->data->t_arr[0], "kill") == 0){
		/* pidwhere indicates the position of pid as argument to kill */
		int pidwhere = 1;
		/* indicate wether -9 flag is supplied */
		int flag = 0;
		if (node->data->t_arr[1] == NULL){
			/* no arguments are supplied */
			printf("kill: usage: -9 (optional) %%jobid or pid\n");
			goto end;
		}
		if (strcmp(node->data->t_arr[1], "-9") == 0){
			if (node->data->t_arr[2]==NULL){
				/* no argument is supplied to kill after flag */
				printf("kill: usage: -9 (optional) %%jobid or pid\n");
				goto end;

			}
			flag = 1;
			pidwhere = 2;
		}
		if (node->data->t_arr[pidwhere][0] == '%'){
			/* job id is supplied  */
                        //check that there is nothing after %jobid
                        //gets pid from jobid
                        char* temp = &node->data->t_arr[pidwhere][1];
                        int jid = atoi(temp);
                        killpid(findnodebyjobid(jid), flag);

		}
		else{
			/* case for pid supplied */
				//check that there is nothing after pid
				//convert to pid
				int pid = atoi(node->data->t_arr[pidwhere]);
				killpid(findnodebypid(pid), flag);
		}
end:;
		/* delete the job containing kill */
		sigset_t new_set, old_set;
		sigemptyset( &new_set );
		sigaddset( &new_set, SIGCHLD );
		sigprocmask( SIG_BLOCK, &new_set, &old_set);
		removenode(node);
		sigprocmask( SIG_SETMASK, &old_set, NULL);
                return 1;

	}
	/*Handle jobs*/
	else if(strcmp(node->data->t_arr[0], "jobs") == 0){
		if (node->data->t_arr[1] != NULL){
			printf(" jobs takes no arguments\n");
		}
		/* remove jobs node */
		sigset_t new_set, old_set;
        	sigemptyset( &new_set );
        	sigaddset( &new_set, SIGCHLD );
        	sigprocmask( SIG_BLOCK, &new_set, &old_set);
		removenode(node);
		sigprocmask( SIG_SETMASK, &old_set, NULL);
		/* print jobs */
		printll();
		return 1;
	}

	/* execute command */
        else{
                int status;
                pid_t pid, wpid;
		pid_t tc_pid;

		/* restore default termois settings */
		if ((tc_pid = tcgetpgrp(STDOUT_FILENO)) < 0)
      			perror("tcgetpgrp() error");

		if (tcgetattr(STDIN_FILENO, &term1) != 0)
			perror("tcgetattr() error\n");

        	pid = fork();
			/* create a child */
        	if (pid == 0){
			sigdefault();
			if (setpgid(getpid(), 0) != 0)
    				perror("setpgid() error");

            /*Child code*/
			if (execvp(node->data->t_arr[0], node->data->t_arr) == -1) {
                                /*if exec doesn't succeed*/
				sigset_t new_set, old_set;
        			sigemptyset( &new_set );
       	 			sigaddset( &new_set, SIGCHLD );
        			sigprocmask( SIG_BLOCK, &new_set, &old_set);
				removenode(node);
				sigprocmask( SIG_SETMASK, &old_set, NULL);
				jobidcounter--;
                                printf("Error executing command!\n");
				node->data->running = 0;
            		}
         
        }
        	else if (pid > 0){
             		sigset_t new_set, old_set;
    			sigemptyset( &new_set );
    			sigaddset( &new_set, SIGCHLD );
       			sigprocmask( SIG_BLOCK, &new_set, &old_set);

				/* populate job paraments */
				/* solve race condition */
			node->data->pid = pid;
			node->data->gid = pid;
			node->data->running = 1;
			sigprocmask( SIG_SETMASK, &old_set, NULL);

			if (node->data->background==1){
				/* do not wait for child to finish */
				printf("[%d]\t %d\n",node->data->jobid,pid);
				/* restore ebash default termios settings */
				if (tcsetpgrp(STDOUT_FILENO, getpgrp()) != 0)
      					perror("tcsetpgrp() error in forked parent when child is bg");
				if (tcsetattr(STDIN_FILENO, TCSANOW, &term1) != 0)
      					perror("tcsetattr() error");
			}

			else{
			 /*Parent waits for child to finish*/
            			do {
					if (tcsetpgrp(STDOUT_FILENO, pid) != 0)
      						perror("tcsetpgrp() error in forked parent when child is fg");
                   			wpid = waitpid(pid, &status, 0);
            			}while(!WIFEXITED(status) && !WIFSIGNALED(status));

			}
			 
        }
        else{
            printf("Error Forking!\n");
        }
        return 1;
    }

}

int fg(struct Node *node){
	//send SIGCONT to the backgrounded job
	pid_t wpid;
	int status;
	node->data->background = 0;
	do {
		/* restore termius setting */
		if (tcsetpgrp(STDIN_FILENO, node->data->pid) != 0)
			perror("tcsetpgrp() error in fg");
		kill(node->data->pid, SIGCONT);
		//if (tcsetattr(STDIN_FILENO, TCSANOW, &(node->data->term)) != 0)
                       // perror("tcsetattr() error");
		wpid = waitpid(node->data->pid, &status, 0);
	}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	return 0;

}

void bg(struct Node *node){
	/* background a job specified by node */
	kill(node->data->pid, SIGCONT);
}

void killpid(struct Node *node, int flag){
	/* kill job specified by node */
	if (flag == 1){
		kill(node->data->pid, SIGKILL);
	}
	else{
		kill(node->data->pid, SIGTERM);
	}
}



