#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define INODEOFFSET (void *)(disk + BLOCKSIZE + sb->inode_offset * BLOCKSIZE)
#define DATAOFFSET  (void *)(disk + BLOCKSIZE + sb->data_offset  * BLOCKSIZE)

#define DELIM "/"
#define MAXLEN 50
#define ROOT 0

int init_flag = 0;
int disksize;
superblock *sb;
void *disk;
int open_fd_table[FD_TABLE_SIZE][FD_MAX];

int f_open(char *);
int f_read(int, int, void*);

/*helper functions*/
int get_inode(char* , int );
int get_block(int );
int get_fd(int );
void init_library(char * );
void read_disk(char * );
void free_and_exit();

int main(){
	//char *filename = malloc(MAXLEN*MAXLEN);
	//filename = "file.txt";
	char filename[MAXLEN] = "usr/file.txt";
	int fd = f_open(filename);
	printf("fd: %d\n",fd);
	return 0;
}
/*returns the inode offset of the directory with name 'name' in datablock with offset 'o'*/
int get_inode(char* name, int o){
	printf("block of dir: %d\n", o);
	dentry *temp = malloc(sizeof(dentry));
	temp = (dentry *)(DATAOFFSET + o);
	int len = strlen(name);
	printf("len: %d; name: %s\n", len, name);
	do{
		printf("temp->n: %d\n", temp->n);
		void *temp_name = malloc(temp->length);
		temp_name = (void *)temp + sizeof(dentry);
		printf("temp_name: %s; temp->size: %d\n", (char *)temp_name, temp->size);
		if (strcmp(name, temp_name) == 0){
			printf("WELL..\n");
			return temp->n;
		}
		if (temp->last == 1)
			break;
		temp = (dentry *)((void *)temp + temp->size);
	}while(1);
	return -1;
}

int get_block(int n){
	printf("inode of dir: %d\n", n);
	inode *temp_inode = malloc(sizeof(inode));
	temp_inode = (inode *) (INODEOFFSET + n);
	int block = temp_inode->dblocks[0];
	//free(temp_inode);
	return block;
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

void init_library(char *d){
	for (int i = 0; i< FD_TABLE_SIZE; i++){
		open_fd_table[i][FD_INODE] = -1;
		open_fd_table[i][FD_SEEK_POS] = 0;
	}
	read_disk(d);
	
}

void read_disk(char *d){
    FILE *fp = fopen(d, "r+");
    if (fp == NULL){
        printf("Disk doesn't exist, please use format.o to make a new one\n");
        exit(1);
    }
    if (fseek(fp, 0, SEEK_END) == -1)
            free_and_exit();
    disksize = ftell(fp);
    if (disksize == -1)
            free_and_exit();
    if (fseek(fp, 0, SEEK_SET) == -1)
            free_and_exit();
    disk = malloc(disksize);
    if (disk == NULL){
        printf("Error in malloc for disksize\n");
        exit(1);
    }
    /*Read disk into buffer*/
    if (fread(disk, 1, disksize, fp) == -1)
            free_and_exit();

    /*read superblock and define block size
     * boot size is assumed to be 512*/
    sb = (superblock*)(disk);
#undef BLOCKSIZE
#define BLOCKSIZE sb->size

    fclose(fp);
    return;
}

void free_and_exit(){
    free(disk);
    printf("an error has occured. Exiting.\n");
    exit(EXIT_FAILURE);
}

int f_open(char * file){
	if (init_flag == 0){
		init_library("DISK");
		init_flag = 1;
	}
	char *token = strtok(file, DELIM);
	printf("here\n");
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
			printf("%s: No such file or directory\n", file);
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
