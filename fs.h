#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define INODEOFFSET (void *)(disk + BLOCKSIZE + sb->inode_offset * BLOCKSIZE)
#define DATAOFFSET  (void *)(disk + BLOCKSIZE + sb->data_offset  * BLOCKSIZE)

#define DELIM "/"
#define MAX_LEN 50
#define ROOT 0

extern char *pwd;

int f_open(char * , int);
int f_read(int, int, void* );
int f_write(void *, int, int );
int f_opendir(char * );
dentry *f_readdir(int );
int f_mkdir(char * );
int f_rmdir(char * );
void init_library(char *);
void close_library();
