#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

superblock *sb;
void *buffer;
FILE *disk;

void create_new(int);
void initialize_newdisk(char *);
void free_and_exit();

int main(int argc, char *argv[]){
	create_new(1);
	return 0;
}

void create_new(int size){
	/*create superblock*/
	sb = malloc(sizeof(superblock));
	sb->size = BLOCKSIZE;
	sb->inode_offset = 0;
	sb->data_offset = 4;
	sb->free_inode = 1;
	sb->free_block = 0;
	printf("Size of sb: %ld\n", sizeof(superblock));
	initialize_newdisk("testdisk");
	if (fseek(disk, 0, SEEK_SET) == -1){
		free_and_exit();
	}
	if (fwrite(sb, 1, sizeof(superblock), disk) != sizeof(superblock)){
		free_and_exit();
	}
	if (fseek(disk, 0, SEEK_END) == -1){
		free_and_exit();
	}
	
	/*fill rest of block (with zeroes?)*/
	void *filler = malloc(sizeof(int));
	*(int *) filler = 0;
	
	if (fwrite(filler, 1, BLOCKSIZE-sizeof(superblock), disk) != BLOCKSIZE-sizeof(superblock)){
        free_and_exit();
    }

	/*creating inodes*/
	if (fseek(disk, 0, SEEK_END) == -1)
		free_and_exit();
	inode *temp;
	temp = malloc(sizeof(inode));
	printf("Size of inode: %ld\n",sizeof(inode));
	temp->nlink = 0;
	temp->size = 0;

	for (int i = 0; i<N_INODES; i++){
		temp->next_inode = i+1;
		if (fwrite(temp, 1, sizeof(inode), disk) != sizeof(inode))
			free_and_exit();
		if (fseek(disk, 0, SEEK_END) == -1)
        	free_and_exit();
	}




	/*-----SUPERBLOCK TEST-------------------------------*/
	buffer = malloc(sizeof(superblock));
	fseek(disk, 0, SEEK_SET);
	fread(buffer, 1, sizeof(superblock), disk);
	superblock *test;
	test = malloc(sizeof(superblock));
	test = (superblock*) buffer;
	printf("-----sb test-------------------------------\n");
	printf("size: %d\n", test->size);
	printf("inode_offset: %d\n", test->inode_offset);
	printf("data_offset: %d\n", test->data_offset);
	printf("-------------------------------------------\n");
	fseek(disk, 0, SEEK_END);
	free(test);
	/*----------------------------------------------------*/
	/*-----INODES TEST------------------------------------*/
	buffer = malloc(sizeof(inode));
	fseek(disk, 512+104, SEEK_SET);
	fread(buffer, 1, sizeof(inode), disk);
	inode *itest = (inode *) buffer;
	printf("-----inode test-------------------------------\n");
    printf("size: %d\n", itest->size);
    printf("nlink: %d\n", itest->nlink);
    printf("next: %d\n", itest->next_inode);
    printf("protect: %d\n", itest->protect);
    printf("-------------------------------------------\n");
	fseek(disk, 0, SEEK_END);
	free(itest);
	/*----------------------------------------------------*/
	
	free(sb);
	fclose(disk);
	
	
}

void initialize_newdisk(char *name){
        disk = fopen(name, "w+");
        if (disk == NULL)
			free_and_exit();
}

void free_and_exit(){
	free(sb);
	fclose(disk);
	exit(1);
}

