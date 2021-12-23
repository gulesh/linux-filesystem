#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define INODEOFFSET (BLOCKSIZE + sb->inode_offset * BLOCKSIZE)
#define DATAOFFSET  (BLOCKSIZE + sb->data_offset  * BLOCKSIZE)

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
	sb->free_inode = 3;
	sb->free_block = 3;
	printf("Size of sb: %ld\n", sizeof(superblock));
	initialize_newdisk("DISK");

	/*malloc for size to get garbage*/
	int bytes = size*1024*1024;
	buffer = malloc(bytes);
	/*fill disk with garbage*/
	if (fseek(disk, 0, SEEK_SET) == -1){
        free_and_exit();
    }
    if (fwrite(buffer, 1, bytes, disk) != bytes){
        free_and_exit();
    }

	/*superblock overrides garbage*/
	if (fseek(disk, 0, SEEK_SET) == -1){
		free_and_exit();
	}
	if (fwrite(sb, 1, sizeof(superblock), disk) != sizeof(superblock)){
		free_and_exit();
	}
	if (fseek(disk, 0, SEEK_END) == -1){
		free_and_exit();
	}
	
	/*NO NEED BECAUSE OF FILLING WITH GARBAGE FROM BEGINNING*/
	/*
	//fill rest of block (with zeroes?)
	void *filler = malloc(sizeof(int));
	*(int *) filler = 0;
	
	if (fwrite(filler, 1, BLOCKSIZE-sizeof(superblock), disk) != BLOCKSIZE-sizeof(superblock)){
        free_and_exit();
    }
	*/

	/*creating inodes*/
	if (fseek(disk, BLOCKSIZE, SEEK_SET) == -1){
		free_and_exit();
	}
	inode *temp;
	temp = malloc(sizeof(inode));
	printf("Size of inode: %ld\n",sizeof(inode));
	temp->nlink = 0;
	temp->size = 0;

	for (int i = 0; i<N_INODES; i++){
		temp->next_inode = i+1;
		if (fwrite(temp, 1, sizeof(inode), disk) != sizeof(inode))
			free_and_exit();
	}

	/*creating mountpoint for home*/
	if (fseek(disk, INODEOFFSET, SEEK_SET) == -1){
        free_and_exit();
    }
	inode *home = malloc(sizeof(inode));
	buffer = malloc(sizeof(inode));
	fread(buffer, 1, sizeof(inode), disk);
	home = (inode*) buffer;
	home->type = MNT_PNT;
	home->nlink = 1;
	
	dentry *home_self = malloc(sizeof(dentry));
	home_self->n = 0;
	home_self->length = 8;
	home_self->type = MNT_PNT;
	home_self->size = (int)sizeof(dentry) + home_self->length;
	home_self->last = 0;
	home->size+=home_self->size;

	/*creating folder for usr inside home*/
	home->dblocks[0] = 0;
	dentry *usr = malloc(sizeof(dentry));
	usr->n = 1;
	usr->length = 8;
	usr->type = DIRECTORY;
	usr->size = (int)sizeof(dentry) + usr->length;
	usr->last = 1;
	home->size+=usr->size;
	/*creating dentry for file called file.txt*/
	dentry *f = malloc(sizeof(dentry));
	f->n = 2;
	f->length = 8;
	f->type = REGULAR;
	f->size = (int)sizeof(dentry) + f->length; 
	f->last = 1;

	if (fseek(disk, INODEOFFSET, SEEK_SET) == -1){
        free_and_exit();
    }
	if (fwrite(home, 1, sizeof(inode), disk) != sizeof(inode))
    	free_and_exit();
	
	free(buffer);
	
	/*writing dentry for . to home's block*/
	if (fseek(disk, DATAOFFSET, SEEK_SET) == -1){
        free_and_exit();
    }
	void *filename = "/";

	printf("ftell: %ld\n", ftell(disk));

	if (fwrite(home_self, 1, sizeof(dentry), disk) != sizeof(dentry))
        free_and_exit();
	if (fwrite(filename, 1, sizeof(filename), disk) != sizeof(filename))
        free_and_exit();

	/*writing dentry for usr to disk*/
	printf("ftell: %ld\n", ftell(disk));
	filename = "usr";
	if (fwrite(usr, 1, sizeof(dentry), disk) != sizeof(dentry))
        free_and_exit();
	if (fwrite(filename, 1, sizeof(filename), disk) != sizeof(filename))
       	free_and_exit();

	
	/*creating dentry for . of usr*/	
	dentry *usr_self = malloc(sizeof(dentry));
    usr_self->n = 1;
    usr_self->length = 8;
    usr_self->type = DIRECTORY;
	usr_self->last = 0;
    usr_self->size = (int)sizeof(dentry) + usr_self->length;

	/*creating the actual node for usr*/
	printf("inode offset: %d\n", INODEOFFSET + (int)sizeof(inode));
    inode *usr_inode = malloc(sizeof(inode));
    buffer = malloc(sizeof(inode));
	if (fseek(disk, INODEOFFSET+(int)sizeof(inode), SEEK_SET) == -1){
        free_and_exit();
    }
    fread(buffer, 1, sizeof(inode), disk);
    usr_inode = (inode*) buffer;
    usr_inode->type = DIRECTORY;
    usr_inode->nlink = 1;
	usr_inode->dblocks[0] = 2;
	usr_inode->size += usr_self->size;
	usr_inode->size+=f->size;
	if (fseek(disk, INODEOFFSET+(int)sizeof(inode), SEEK_SET) == -1){
        free_and_exit();
    }
	if (fwrite(usr_inode, 1, sizeof(inode), disk) != sizeof(inode))
        free_and_exit();

    free(buffer);

	/*writing . in usr's data*/
	if (fseek(disk, DATAOFFSET+2*BLOCKSIZE, SEEK_SET) == -1){
        free_and_exit();
    }
    filename = ".";

    printf("ftell: %ld\n", ftell(disk));

    if (fwrite(usr_self, 1, sizeof(dentry), disk) != sizeof(dentry))
        free_and_exit();
    if (fwrite(filename, 1, sizeof(filename), disk) != sizeof(filename))
        free_and_exit();
	/*writing dentry for file.txt to disk*/
    printf("ftell: %ld\n", ftell(disk));
    filename = "file.txt";
    if (fwrite(f, 1, sizeof(dentry), disk) != sizeof(dentry))
        free_and_exit();
    if (fwrite(filename, 1, sizeof(filename), disk) != sizeof(filename))
        free_and_exit();


	/*creating the actual node for file.txt*/
	inode *f_inode = malloc(sizeof(inode));
    buffer = malloc(sizeof(inode));
	if (fseek(disk, INODEOFFSET+(2*(int)sizeof(inode)), SEEK_SET) == -1){
        free_and_exit();
    }
    fread(buffer, 1, sizeof(inode), disk);
    f_inode = (inode*) buffer;
    f_inode->type = REGULAR;
    f_inode->nlink = 1;
    f_inode->dblocks[0] = 1;
	f_inode->size += 80;
    if (fseek(disk, INODEOFFSET+(2*(int)sizeof(inode)), SEEK_SET) == -1){
        free_and_exit();
    }
    if (fwrite(f_inode, 1, sizeof(inode), disk) != sizeof(inode))
        free_and_exit();

    free(buffer);

	/*filling file.txt with content*/
	char *content = malloc(20);
	strcpy(content, "This is a test file.");
	if (fseek(disk, DATAOFFSET + BLOCKSIZE, SEEK_SET) == -1){
        free_and_exit();
    }
	for (int i = 0; i<2; i++){
		if (fwrite(content, 1, strlen(content), disk) != strlen(content))
        	free_and_exit();
	}
	content[4]  = '\n';
	content[7] = '\n';
	for (int i = 0; i<2; i++){
		if (fwrite(content, 1, strlen(content), disk) != strlen(content))
        	free_and_exit();
	}
	/*filling free data blocks*/
	if (fseek(disk, DATAOFFSET + 3*BLOCKSIZE, SEEK_SET) == -1){
        free_and_exit();
    }
	int to_fill = size*2042-8;
	int cur = 4;
	void *nextfree = malloc(sizeof(int));
	printf("TOFILL: %d\n", to_fill);
	for (int i = 0; i<to_fill-1; i++){
		if (fseek(disk, DATAOFFSET + (cur-1)*BLOCKSIZE, SEEK_SET) == -1){
        	free_and_exit();
    	}
		*(int *) nextfree = cur;
		fwrite(nextfree, 1, sizeof(int), disk);
		cur++;
	}
	if (fseek(disk, DATAOFFSET + (cur-1)*BLOCKSIZE, SEEK_SET) == -1){
		free_and_exit();
    }
    *(int *) nextfree = -1;
   	fwrite(nextfree, 1, sizeof(int), disk);
	free(nextfree);

	

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

	/*-----INODES & DENTRY TESTS--------------------------*/
	buffer = malloc(sizeof(inode));
	fseek(disk, 512+104, SEEK_SET);
	fread(buffer, 1, sizeof(inode), disk);
	inode *itest = (inode *) buffer;
	printf("-----inode test----------------------------\n");
    printf("size: %d\n", itest->size);
    printf("nlink: %d\n", itest->nlink);
    printf("next: %d\n", itest->next_inode);
    printf("type: %d\n", itest->type);
    printf("-------------------------------------------\n");

	printf("-----dentry test----------------------------\n");
	printf("first data block: %d\n", itest->dblocks[0]);
	for (int j = 0; j<3; j+=2){
		fseek(disk, DATAOFFSET + j * BLOCKSIZE, SEEK_SET);
		free(buffer);
		buffer = malloc(sizeof(dentry));
		for (int i = 0; i<2; i++){
		
			printf("ftell: %ld\n", ftell(disk));
			fread(buffer, 1, sizeof(dentry), disk);
			dentry *dtest = (dentry *) buffer;
    		printf("inode:  %d\n", dtest->n     );
			printf("size:   %d\n", dtest->size  );
    		printf("type:   %d\n", dtest->type  );
    		printf("length: %d\n", dtest->length);
    		printf("last:   %d\n", dtest->last  );
			int len = dtest->length;
			free(buffer);
			buffer = malloc(len);
			fread(buffer, 1, len, disk);
			//fseek(disk, DATAOFFSET+sizeof(dentry)+8, SEEK_SET);
    		printf("dentry name: %s\n", (char*)buffer);
			printf("-----\n");
		}
	}
    printf("-------------------------------------------\n");
	free(buffer);
	/*----------------------------------------------------*/
	
	/*-----FILE TEST------------------------------------*/
	fseek(disk, DATAOFFSET+BLOCKSIZE, SEEK_SET);
	buffer = malloc((long)80);
	fread(buffer, 1, 80, disk);
	printf("Contents of file: %s\n", (char *)buffer);
	free(buffer);
	/*----------------------------------------------------*/

	/*-----FREE BLOCKS TEST-------------------------------*/

	for (int i = sb->free_block; i< 2037; i++){
		fseek(disk, DATAOFFSET+i*BLOCKSIZE, SEEK_SET);
		buffer = malloc(sizeof(int));
		fread(buffer, 1, sizeof(int), disk);
	//	printf("Block %d is pointing at block %d\n", i, *(int *)buffer);
		free(buffer);
	}
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

