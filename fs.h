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
int f_open(char * );
int f_read(int, int, void* );
int f_opendir(char * );
dentry *f_readdir(int );
