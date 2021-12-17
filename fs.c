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
static void update_open_inode(int );
static inode *get_open_inode(int );
static void update_disk();


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
		printf("fd: %d\tinode: %d\tseek: %d\n", i, 
			open_fd_table[i][FD_INODE],open_fd_table[i][FD_SEEK_POS]);
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
	dentry *temp = (dentry *)(DATAOFFSET + o*BLOCKSIZE);
	do{
		void *temp_name = (void *)temp + sizeof(dentry);
		if (memcmp(name, temp_name, strlen(name)) == 0){
			int ret = temp->n;
			return ret;
		}
		if (temp->last == 1)
			break;
		temp = (dentry *)((void *)temp + temp->size);
	}while(1);
	return -1;
}

static int get_block(int n){
	inode *temp_inode = (inode *) (INODEOFFSET + n*(int)sizeof(inode));
	int block = temp_inode->dblocks[0];
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

    /*read superblock and define block size*/
    sb = (superblock*)(disk);
#undef BLOCKSIZE
#define BLOCKSIZE sb->size

    fclose(fp);
    return;
}

static void free_and_exit(){
    close_library();
    printf("an error has occured. Exiting.\n");
    exit(EXIT_FAILURE);
}

int f_open(char * file, int mode){
	if (strcmp(file, "/") == 0){
		return 0;
	}
	else{

		char *token = strtok(file, DELIM);
		char tokens[MAX_LEN][MAX_LEN];
		int idx = 0;
		// loop through the string to extract all other tokens
		while( token != NULL ) {
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
		int f_inode;
		if (mode == 1){
			/*new i_node*/
			f_inode = sb->free_inode;
			inode *creat = (inode *)(INODEOFFSET + 
				f_inode*(int)sizeof(inode));
			sb->free_inode = creat->next_inode;
			creat->nlink = 1;
			creat->size = 0;
			creat->type = REGULAR;
			creat->dblocks[0] = sb->free_block;
			sb->free_block = *(int*) (DATAOFFSET + sb->free_block*BLOCKSIZE);
			
			/*new dentry*/
			dentry *t = (dentry *)(DATAOFFSET + curr_d*BLOCKSIZE);
			int count = 0;
    		do{
        		if (t->last == 1){
					t->last = 0;
					count += t->size;
            		break;
				}
				count += t->size;
        		t = (dentry *)((void *)t + t->size);
    		}while(1);
        	
			t = (dentry *)((void *)t + t->size);
			t->n = f_inode;
			t->type = REGULAR;
			t->length = strlen(tokens[idx-1]);
			t->last = 1;
			t->size = (int)sizeof(dentry)+t->length;
			
			//memcpy(DATAOFFSET + curr_d*BLOCKSIZE 
			//	+ count, (void *)t, sizeof(dentry));
			/*update parent size*/
			inode *parent = (inode *)(INODEOFFSET 
				+ curr_i*(int)sizeof(inode));	
			parent->size+=(int)sizeof(dentry);
			memcpy(DATAOFFSET + curr_d*BLOCKSIZE+parent->size, 
				(void*)tokens[idx-1], strlen(tokens[idx-1]));
			parent->size+=strlen(tokens[idx-1]);
			update_open_inode(curr_i);

		}
		else{
			f_inode = get_inode(tokens[idx-1], curr_d);
			if(f_inode == -1){
				printf("%s: No such file or directory\n", file);
				return -1;
			}
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
static void update_open_inode(int n){
	for (int i = 0; i<MAX_OPEN; i++){
        if (inode_table[i]->n == n){
            memcpy((void *)inode_table[i]->ptr, 
			INODEOFFSET + n*sizeof(inode),
			sizeof(inode));
			break;
        }
    }
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
	int fd = f_open(dir, 0);
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
	int f = sb->free_inode;
	inode *new_inode = (inode *) (INODEOFFSET + f*(int)sizeof(inode));
	new_inode->type = DIRECTORY;
	new_inode->size = 0;
	new_inode->nlink = 1;
	new_inode->dblocks[0] = sb->free_block;
	sb->free_block = *(int*) (DATAOFFSET + sb->free_block*BLOCKSIZE);
	
	/*dentry for new dir*/
	dentry *new_dir = malloc(sizeof(dentry));
	new_dir->n = sb->free_inode;
	sb->free_inode = new_inode->next_inode;
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
	dentry *t = (dentry *)(DATAOFFSET + block*BLOCKSIZE);
    do{
        if (t->last == 1){
			t->last = 0;
            break;
		}
        t = (dentry *)((void *)t + t->size);
    }while(1);
	cur->size += (int)sizeof(dentry);
	/*copy name of new dir*/
	memcpy(DATAOFFSET+block*BLOCKSIZE+cur->size, (void*) name, strlen(name));
	cur->size += strlen(name);
	

	/*copy self into new_inode dblocks*/
	memcpy(DATAOFFSET+new_inode->dblocks[0]*BLOCKSIZE, (void*) self, sizeof(dentry));
	void *self_name = ".";
	memcpy(DATAOFFSET+new_inode->dblocks[0]*BLOCKSIZE+sizeof(dentry), 
	(void*) self_name, sizeof(self_name));
	
	update_open_inode(new_dir->n);

	free(new_dir);
	free(self);
	return 0;
}


int f_rmdir(char* path){
	
	char *temp_path = malloc(MAX_LEN);
	strcpy(temp_path, path);
	int temp_wd = working_dir;
	int fd = f_opendir(temp_path);
	free(temp_path);
	working_dir = temp_wd;
	path[strlen(path) - 1] = '\0';
	if (fd == -1)
		printf("no such folder\n");
	int n = open_fd_table[fd][FD_INODE];
	inode *rm_inode = get_open_inode(n);
	/*check if folder is not empty*/
	if (rm_inode->size>(int)sizeof(dentry)+8){
		printf("Can't remove dir; not empty.\n");
		return(-1);
	}
	/*add dir block to the pool of free blocks*/
	int block = rm_inode->dblocks[0];
	void *curr_free = malloc(sizeof(int));
	*(int *) curr_free = sb->free_block;
	memcpy(DATAOFFSET+block*BLOCKSIZE, curr_free, sizeof(int));
	free(curr_free);
	sb->free_block = block;
	/*add dir inode to the pool of free inodes*/
	rm_inode->next_inode = sb->free_inode;
	rm_inode->nlink = 0;
	rm_inode->size = 0;
	sb->free_inode = n;
	/*remove dentry from parent inode*/
	int count = 0;
	int i = 0;
	while(1){
		char temp = path[i];
		if (temp == '/')
			count=i;
		if (temp == '\0')
			break;
		i++;
	}
	int fd_parent;
	if (count == 0)
		fd_parent = 0;

	else{
		char *parent_path = malloc(count + 1);
		memcpy(parent_path, path, count);
		parent_path[count] = '\0';
		fd_parent = f_opendir(parent_path);
		free(parent_path);
		working_dir = temp_wd;
	}
	int n_parent = open_fd_table[fd_parent][FD_INODE];
	inode *parent_inode = get_open_inode(n_parent);
	int parent_block = parent_inode->dblocks[0];
	dentry *temp = (dentry *)(DATAOFFSET + parent_block*BLOCKSIZE);
	dentry *prev = (dentry *)(DATAOFFSET + parent_block*BLOCKSIZE);
    do{
        if (temp->n == n){
			if(temp->last == 1){
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

int f_write(void *buffer, int size, int fd){
    int n = open_fd_table[fd][FD_INODE];
	if (n==-1)
		return 0;
	int to_paste = size-1;
	int pasted = 0;
    inode *file_inode = (inode *)(INODEOFFSET + n*(int)sizeof(inode));
    if (file_inode->type == DIRECTORY || file_inode->type == MNT_PNT){
        printf("Not a file\n");
        return 0;
    }
    
	if (to_paste-BLOCKSIZE >= 0){
		memcpy(DATAOFFSET+file_inode->dblocks[0]*BLOCKSIZE,
			buffer + pasted, BLOCKSIZE);
		pasted += BLOCKSIZE;
		to_paste-=BLOCKSIZE;
    }
	else{
		memcpy(DATAOFFSET+file_inode->dblocks[0]*BLOCKSIZE,
        	buffer, to_paste);
		pasted += to_paste;
		to_paste -= to_paste;
		file_inode->size += pasted;
		update_open_inode(n);
		return pasted;
	}
	for (int i = 1; i<N_DBLOCKS; i++){
		if (to_paste <= 0)
			break;
		file_inode->dblocks[i] = sb->free_block;
		sb->free_block = *(int*) (DATAOFFSET + sb->free_block*BLOCKSIZE);
		if (to_paste-BLOCKSIZE >= 0){
			memcpy(DATAOFFSET+file_inode->dblocks[i]*BLOCKSIZE, 
				buffer + pasted, BLOCKSIZE);
			pasted += BLOCKSIZE;
			to_paste-=BLOCKSIZE;
		}	
		else{
			memcpy(DATAOFFSET+file_inode->dblocks[i]*BLOCKSIZE,
        		buffer + pasted, to_paste);
			pasted+=to_paste;
			to_paste-=to_paste;
			file_inode->size += pasted;
			update_open_inode(n);
			return pasted;
		}


	}
	file_inode->size += pasted;
	update_open_inode(n);
	return pasted;
	
}
static void update_disk(){
	for (int i = 0; i<MAX_OPEN; i++){
        if (inode_table[i]->n != -1){
            memcpy((void *)inode_table[i]->ptr, 
			INODEOFFSET + inode_table[i]->n*sizeof(inode),
			sizeof(inode));
			break;
        }
    }

}

void close_library(){
	update_disk();
	for (int i = 0; i<MAX_OPEN; i++){
        free(inode_table[i]->ptr);
        free(inode_table[i]);
    }
	free(pwd);
	FILE *fd = fopen("DISK", "w");
	fwrite(disk, disksize, 1, fd);
	fclose(fd);
	free(disk);
	
}






