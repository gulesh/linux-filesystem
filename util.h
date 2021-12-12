#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define BLOCKSIZE 512
#define N_INODES  10 

#define REGULAR   0
#define DIRECTORY 1
#define MNT_PNT   2


/* open file descriptor table definitions */
enum open_fd_table_enum {FD_INODE, FD_SEEK_POS, FD_MAX};
#define FD_TABLE_SIZE 100


/* file descriptor array */
extern int open_fd_table[ FD_TABLE_SIZE ][ FD_MAX ];


typedef struct superblock {
  	int size;
  	int inode_offset;
  	int data_offset;
  	int swap_offset;
  	int free_inode;
  	int free_block;
} superblock;

typedef struct inode {
	int next_inode;
	int type;
	int protect;
 	int nlink;
  	int size;
	int uid;
	int gid;
	int ctime;
	int mtime;
	int atime;
	int dblocks[N_DBLOCKS];
	int iblocks[N_IBLOCKS];
	int i2block;
	int i3block;
} inode;

typedef struct dentry {
	int n;      //inode number
	int size;   //size to offset to next dentry
	int type;
	int length; //file name length
} dentry;

#endif
