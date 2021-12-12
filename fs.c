#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define INODEOFFSET (BLOCKSIZE + sb->inode_offset * BLOCKSIZE)
#define DATAOFFSET  (BLOCKSIZE + sb->data_offset  * BLOCKSIZE)

#define DELIM "/"
#define MAXLEN 20
#define ROOT 0

int init_flag = 0;
extern int open_fd_table[FD_TABLE_SIZE][FD_MAX];

int f_open(char *);
int f_read(int, int, void*);

int main(){
	f_open("file.txt");
	return 0;
}
/*returns the inode offset of the directory with name 'name' in datablock with offset 'o'*/
int get_inode(char* name, int o){
	dentry *temp = malloc(sizeof(dentry);
	temp = (dentry *)(DATAOFFSET + o);
	int len = strlen(name);
	printf("len: %d\n", len);
	do{
		if (len == temp->length){
			char temp_name[temp->length];
			temp_name = (char *) (&temp + sizeof(dentry));
			if (strcmp(name, temp_name) == 0){
				return temp->n;
			}
		}
		temp = (dentry *)(&temp + temp->size);
	}while(temp->last != 1);
	return -1;
}

int get_block(int n){
	inode *temp_inode = (inode *) (INODEOFFSET + n);
	return temp_inode->dblocks[0];
}

int get_fd(int n){
	for (int i = 0; i< FD_TABLE_SIZE; i++){
		if (open_fd_table[i][FD_INODE] == -1){
        	open_fd_table[i][FD_INODE] = n;
			return i;
		}
	}
	return -1;
}

void init_library(){
	for (int i = 0; i< FD_TABLE_SIZE; i++){
		open_fd_table[i][FD_INODE] = -1;
		open_fd_table[i][FD_SEEK_POS] = 0;
	}
}

int f_open(char * file){
	if (init_flag == 0){
		init_library();
		init_flag = 1;
	}
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
	/*current data offset (that of root)*/
	int curr_d = 0;
	/*current inode offset (that of node)*/
	int curr_i = 0;
	for (int i = 0; i<idx-1; i++){
		if(curr_i = get_inode(tokens[i], curr_d) == -1){
			printf("%s: No such file or directory\n", name);
			exit(EXIT_FAILURE);
		}
		if(curr_d = get_block(curr_i) == -1)
			exit(EXIT_FAILURE);
	}
	int f_inode = get_inode(tokens[idx-1], curr_d);
	int fd = get_fd(f_inode);
	if (fd == -1){
		printf("Open fd table is full\n");
		exit(EXIT_FAILURE);
	}
	return fd;
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
