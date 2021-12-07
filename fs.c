#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

int f_open(char *);
int f_read(int, int, void*);

int f_open(char * file){
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
}
