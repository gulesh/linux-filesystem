#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define DELIM "/"
#define MAXLEN 20

int f_open(char *);
int f_read(int, int, void*);

int main(){
	f_open("/usr/usr2/file.txt");
	return 0;
}

int f_open(char * file){
	char *token = strtok(file, DELIM);
	char tokens[MAXLEN][MAXLEN];
	int idx = 0;
	// loop through the string to extract all other tokens
   	while( token != NULL ) {
      	printf( " %s\n", token ); //printing each token
		strcpy(tokens[idx], token);
		idx++;
      	token = strtok(NULL, DELIM);
   	}
	// Print the list of tokens
    printf("Token List:\n");
    for (int i=0; i < idx; i++) {
        printf("%s\n", tokens[i]);
    }
	return 0;
	/*search dentry based on name*/
	/*retreive inode number*/
	/*choose the lowest unused fd*/
	/*add the file to the open file descriptor table, with the fd from above, 
	inode from above, and seek position = 0; switch the tables row value to used.*/
}

int f_read(int fd, int bytes, void* buffer){
	/*get inode from table using fd*/
	/*got to data block and seek to position from table*/
	/*read the required bytes and error if it violates the file size*/
	/*put the bytes in buffer*/
	/*return the number of bytes read*/
	return 0;
}
