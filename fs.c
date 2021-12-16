#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "fs.h"

int init_flag = 0;
int disksize;
superblock *sb;
void *disk;
int open_fd_table[FD_TABLE_SIZE][FD_MAX];
inode_entry* inode_table[MAX_OPEN];
/*inode number of working directory*/
int working_dir;
char *pwd;

/*helper functions*/
static int get_inode(char* , int );
static int get_block(int );
static int get_fd(int );
static void read_disk(char * );
static void free_and_exit();
static void print_open_fd();
static int add_to_inode_table(int );
static void print_open_inodes();


/*static int main(){
	//char *filename = malloc(MAXLEN*MAXLEN);
	//filename = "file.txt";
	char filename[MAXLEN] = "usr/file.txt";
	int fd = f_open(filename);
	char filename2[MAXLEN] = "././usr";
	int fd2 = f_open(filename2);
	printf("fd: %d\n",fd);
	printf("fd2: %d\n",fd2);
	print_open_fd();
	print_open_inodes();
	//Testing f_read
	int to_read = 80;
	void *buf = malloc(to_read);
	f_read(fd,to_read,buf);
	printf("%s\n", (char*) buf);
	return 0;
}*/

static void print_open_fd(){
	for (int i = 0; i<FD_TABLE_SIZE; i++){
		printf("fd: %d\tinode: %d\tseek: %d\n", i, open_fd_table[i][FD_INODE], 
		open_fd_table[i][FD_SEEK_POS]);
	}
}

static void print_open_inodes(){
	for (int i = 0; i< MAX_OPEN; i++){
		printf("data block of node: %d\n", 
		inode_table[i]->ptr->dblocks[0]);
	}
}
/*returns the inode offset of the directory with name 'name' in datablock with offset 'o'*/
static int get_inode(char* name, int o){
	dentry *temp = malloc(sizeof(dentry));
	temp = (dentry *)(DATAOFFSET + o*BLOCKSIZE);
	int len = strlen(name);
	do{
		void *temp_name = malloc(temp->length);
		temp_name = (void *)temp + sizeof(dentry);
		if (strcmp(name, temp_name) == 0){
			return temp->n;
		}
		if (temp->last == 1)
			break;
		temp = (dentry *)((void *)temp + temp->size);
	}while(1);
	return -1;
}

static int get_block(int n){
	inode *temp_inode = malloc(sizeof(inode));
	temp_inode = (inode *) (INODEOFFSET + n*(int)sizeof(inode));
	int block = temp_inode->dblocks[0];
	//free(temp_inode);
	return block;
}

static int get_fd(int n){
	for (int i = 0; i< FD_TABLE_SIZE; i++){
		if (open_fd_table[i][FD_INODE] == -1){
        	open_fd_table[i][FD_INODE] = n;
			return i;
		}
	}
	return -1;
}

static int add_to_inode_table(int n){
	for (int i = 0; i<MAX_OPEN; i++){
		if (inode_table[i]->n == -1){
			inode_table[i]->n = n;
			memcpy(inode_table[i]->ptr, INODEOFFSET + n*(int)sizeof(inode), sizeof(inode));
			return 0;
		}
    }
	printf("No space in inode_table");
	return -1;
}

void init_library(char *d){
	working_dir = 0;
	pwd = malloc(MAX_LEN*MAX_LEN);
	strcpy(pwd, "/");
	open_fd_table[0][FD_INODE] = 0;
	open_fd_table[0][FD_SEEK_POS] = 0;
	for (int i = 1; i< FD_TABLE_SIZE; i++){
		open_fd_table[i][FD_INODE] = -1;
		open_fd_table[i][FD_SEEK_POS] = 0;
	}
	read_disk(d);
	inode_table[0] = malloc(sizeof(inode_entry));
    inode_table[0]->n = 0;
    inode_table[0]->ptr = malloc(sizeof(inode));
	memcpy(inode_table[0]->ptr, INODEOFFSET, sizeof(inode));
	for (int i = 1; i<MAX_OPEN; i++){
		inode_table[i] = malloc(sizeof(inode_entry));
		inode_table[i]->n = -1;
		inode_table[i]->ptr = malloc(sizeof(inode));
	}
	/*inode_entry *home_entry = inode_table[0];
	home_entry->n = 0;
	void *home_inode = (void*)(inode *) home_entry->ptr;
	memcpy(home_inode, INODEOFFSET, sizeof(inode));
	inode *test = (inode *)home_inode;
	printf("test: %d\n",test->size);*/
	
}

static void read_disk(char *d){
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

static void free_and_exit(){
    free(disk);
    printf("an error has occured. Exiting.\n");
    exit(EXIT_FAILURE);
}

int f_open(char * file){
	if (init_flag == 0){
		init_library("DISK");
		init_flag = 1;
	}
	if (strcmp(file, "/") == 0){
		return 0;
	}
	else{

		char *token = strtok(file, DELIM);
		char tokens[MAX_LEN][MAX_LEN];
		int idx = 0;
		// loop through the string to extract all other tokens
		while( token != NULL ) {
			//printf( " %s\n", token ); //printing each token
			strcpy(tokens[idx], token);
			idx++;
			token = strtok(NULL, DELIM);
		}
		/*current data offset (that of root)*/
		int curr_d = 0;
		/*current inode offset (that of node)*/
		int curr_i;
		for (int i = 0; i<idx-1; i++){
			curr_i = get_inode(tokens[i], curr_d);
			if(curr_i == -1){
				printf("%s: No such file or directory\n", file);
				return -1;
			}
			curr_d = get_block(curr_i);
			if(curr_d == -1)
				return -1;
		}
		int f_inode = get_inode(tokens[idx-1], curr_d);
		if(f_inode == -1){
			printf("%s: No such file or directory\n", file);
			return -1;
		}
		int fd = get_fd(f_inode);
		if (fd == -1){
			printf("Open fd table is full\n");
			return -1;
		}
		/*Add the inode to the open inode table*/
		int status = add_to_inode_table(f_inode);
		if (status == -1)
			exit(EXIT_FAILURE);
		return fd;
	}
	/*search dentry based on name*/
	/*retreive inode number*/
	/*choose the lowest unused fd*/
	/*add the file to the open file descriptor table, with the fd from above, 
	inode from above, and seek position = 0; switch the tables row value to used.*/
}

static inode *get_open_inode(int n){
    for (int i = 0; i<MAX_OPEN; i++){
        if (inode_table[i]->n == n){
            return inode_table[i]->ptr;
        }
    }
    return NULL;
}

int f_read(int fd, int bytes, void* buffer){
	int n = open_fd_table[fd][FD_INODE];
	int seek_pos   = open_fd_table[fd][FD_SEEK_POS];
	int first_block = (int)(seek_pos/BLOCKSIZE);
	int offset = seek_pos - first_block*BLOCKSIZE;
	int to_copy = bytes;

	inode *file_inode = get_open_inode(n);
	void *buffer_pos = buffer;

	int remaining = file_inode->size;

	if (seek_pos >= file_inode->size)
		return 0;
	/*copy direct data blocks, if any*/
	for (int i = 0; i < N_DBLOCKS; i++){
		if (remaining <= 0 || to_copy <= 0)
			break;
		if (i>=first_block){
			int copied;
			if (to_copy >= BLOCKSIZE-offset){
				copied = BLOCKSIZE-offset;
			}
			else{
				copied = to_copy;
			}
			memcpy(buffer_pos, DATAOFFSET+file_inode->dblocks[i]*BLOCKSIZE+offset, copied);
			buffer_pos+=copied;
			to_copy-=copied;
			remaining-=copied;
			offset = 0;
		}
	}
	for (int i = 0; i<N_IBLOCKS; i++){
		if (remaining <= 0 || to_copy <= 0)
            break;
        if (N_DBLOCKS+i>=first_block){
			void *iblock = DATAOFFSET+file_inode->iblocks[i]*BLOCKSIZE;
			for (int j = 0; j < BLOCKSIZE; j+=4){
				if (remaining <= 0 || to_copy <= 0)
            		break;
				if (N_DBLOCKS + i*(j/4*BLOCKSIZE)>=first_block){	
					int inner = *((int *) iblock+j);
					int copied;
					if (to_copy >= BLOCKSIZE-offset){
						copied = BLOCKSIZE-offset;
					}
					else{
						copied = to_copy;
					}
					memcpy(buffer_pos, DATAOFFSET+inner*BLOCKSIZE+offset, copied);
					buffer_pos+=copied;
					to_copy-=copied;
					remaining-=copied;
					offset = 0;
				}


			}
		}
	}
	open_fd_table[fd][FD_SEEK_POS] = seek_pos + bytes;
	
	/*get inode from table using fd*/
	/*got to data block and seek to position from table*/
	/*read the required bytes and error if it violates the file size*/
	/*put the bytes in buffer*/
	/*return the number of bytes read*/
	return bytes-to_copy;
}

int f_opendir(char *dir){
	int fd = f_open(dir);
	working_dir = open_fd_table[fd][FD_INODE];
	return fd;
}

dentry *f_readdir(int fd){
	int n = open_fd_table[fd][FD_INODE];
    int seek_pos = open_fd_table[fd][FD_SEEK_POS];
	inode *dir_inode = get_open_inode(n);
	if (dir_inode->type == REGULAR){
		printf("Not a directory\n");
		return NULL;
	}
	
	if (seek_pos >= dir_inode->size){
		open_fd_table[fd][FD_SEEK_POS] = 0;
		return NULL;
	}

	dentry *temp; //malloc?
    temp = (dentry *)(DATAOFFSET + dir_inode->dblocks[0]*BLOCKSIZE + seek_pos);
	open_fd_table[fd][FD_SEEK_POS] = seek_pos + temp->size;
	return temp;
}

int f_mkdir(char *name){
	inode *cur = get_open_inode(working_dir);
	int block = cur->dblocks[0];
	/*inode for new dir*/
	printf("free_inode: %d\n", sb->free_inode);
	printf("free_block: %d\n", sb->free_block);
	int f = sb->free_inode;
	inode *new_inode = (inode *) (INODEOFFSET + f*(int)sizeof(inode));
	printf("next: %d\n", new_inode->next_inode);
	new_inode->type = DIRECTORY;
	new_inode->nlink = 1;
	new_inode->dblocks[0] = sb->free_block;
	sb->free_block = *(int*) (DATAOFFSET + sb->free_block*BLOCKSIZE);
	printf("free_block: %d\n", sb->free_block);
	
	/*dentry for new dir*/
	dentry *new_dir = malloc(sizeof(dentry));
	new_dir->n = sb->free_inode;
	sb->free_inode = new_inode->next_inode;
	printf("free_inode: %d\n", sb->free_inode);
	new_dir->length = strlen(name); 
	new_dir->type = DIRECTORY;
	new_dir->size = (int)sizeof(dentry) + new_dir->length;
	new_dir->last = 1;

	/*dentry for . of this dir*/
	dentry *self = malloc(sizeof(dentry));
	self->n = new_dir->n;
	self->length = 8;
	self->type = DIRECTORY;
	self->size = (int)sizeof(dentry)+self->length;
	self->last = 1;
	new_inode->size+=self->size;

	/*copy new_dir to the end of the block of the current directory*/
	memcpy(DATAOFFSET+block*BLOCKSIZE+cur->size, (void *)new_dir, sizeof(dentry));
	/*change last dentry before mkdir to have last = 0*/
	dentry *temp = (dentry *)(DATAOFFSET + block*BLOCKSIZE);
    do{
        if (temp->last == 1){
			temp->last = 0;
            break;
		}
        temp = (dentry *)((void *)temp + temp->size);
    }while(1);

	dentry *test = (dentry*)(DATAOFFSET+block*BLOCKSIZE+cur->size);

	cur->size += sizeof(dentry);
	/*copy name of new dir*/
	memcpy(DATAOFFSET+block*BLOCKSIZE+cur->size, (void*) name, strlen(name));
	cur->size += strlen(name);
	

	/*copy self into new_inode dblocks*/
	memcpy(DATAOFFSET+new_inode->dblocks[0]*BLOCKSIZE, (void*) self, sizeof(dentry));
	void *self_name = ".";
	memcpy(DATAOFFSET+new_inode->dblocks[0]*BLOCKSIZE+sizeof(dentry), 
	(void*) self_name, sizeof(self_name));

	return 0;
}


int f_rmdir(char* path){
	printf("(FROM LIB) path is %s\n", path);
	
	char *temp_path = malloc(MAX_LEN);
	strcpy(temp_path, path);
	int fd = f_opendir(temp_path);
	free(temp_path);
	path[strlen(path) - 1] = '\0';
	if (fd == -1)
		printf("");
	int n = open_fd_table[fd][FD_INODE];
	inode *rm_inode = get_open_inode(n);
	printf("(FROM LIB) n = %d\n", n);
	/*check if folder is not empty*/
	
	/*add dir block to the pool of free blocks*/
	int block = rm_inode->dblocks[0];
	void *curr_free = malloc(sizeof(int));
	*(int *) curr_free = sb->free_block;
	memcpy(DATAOFFSET+block*BLOCKSIZE, curr_free, sizeof(int));
	sb->free_block = block;
	/*add dir inode to the pool of free inodes*/
	rm_inode->next_inode = sb->free_inode;
	rm_inode->nlink = 0;
	rm_inode->size = 0;
	sb->free_inode = n;
	printf("(FROM LIB) free_inode is  %d; free_block is %d\n", 
		sb->free_inode, sb->free_block);
	printf("(FROM LIB) next is  %d\n", rm_inode->next_inode); 
	/*remove dentry from parent inode*/
	int count = 0;
	int i = 0;
	while(1){
		char temp = path[i];
		printf("(FROM LIB) %d:%c\n", i, temp);
		if (temp == '/')
			count=i;
		if (temp == '\0')
			break;
		i++;
	}
	printf("count = %d\n", count);
	int fd_parent;
	if (count == 0)
		fd_parent = 0;

	else{
		char *parent_path = malloc(count + 1);
		memcpy(parent_path, path, count);
		parent_path[count] = '\0';
		printf("parent_path: %s\n", parent_path);
		fd_parent = f_opendir(parent_path);
	}
	int n_parent = open_fd_table[fd_parent][FD_INODE];
	inode *parent_inode = get_open_inode(n_parent);
	int parent_block = parent_inode->dblocks[0];
	printf("(FROM LIB) n_parent is  %d; parent_block is %d\n", 
		n_parent, parent_block);
	dentry *temp = (dentry *)(DATAOFFSET + parent_block*BLOCKSIZE);
	dentry *prev = (dentry *)(DATAOFFSET + parent_block*BLOCKSIZE);
    do{
        if (temp->n == n){
			if(temp->last == 1){
				printf("(FROM LIB) temp->n: %d; prev->n: %d\n", temp->n, prev->n); 
				prev->last = 1;
				parent_inode->size-=temp->size;
			}
			else{
				prev->size+=temp->size;
			}
            break;
        }
		prev = temp;
        temp = (dentry *)((void *)temp + temp->size);
    }while(1);

	return 0;
}



