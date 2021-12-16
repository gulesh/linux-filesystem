# HW7

### Gulesh, Ivan, and Mohamed

## Compilation

Compile the shell link to library using **make**. remove excuetables
with make clean.  Explore other shortcuts for compiling the library, the
shell, and the format utility separately in the Makefile. Run using
./mysh. Compile the format utility using make format.  Run using
./format.exec.


## Basic Structure

A 1 mb disk is present and is mounted automatically. The disk contains
the root mount point, a directory inside it named usr, and a sample test
file inside usr called file.text.  The structure of the file system is
very similar to the disk from HW6, with 10 direct blocks, 4 singly
indirect, 1 doubly and one triply. The signatures of the library
functions are shown below.


```
int f_open(char * , int);
/* returns a file number of the open file */

int f_read(int fileno, int number_of_bytes, 
		void* pointer_to_read_buffer );
int f_open(char * , int);
int f_write(void *, int, int );
int f_opendir(char * );
dentry *f_readdir(int );
int f_mkdir(char * );
int f_rmdir(char * );
void init_library(char *);
void close_library();
```


We keep 3 global data structures shared between the shell and the
library

## Implemented Features

## Tests

## Limitations

## Problematic Features

## Memory Management
