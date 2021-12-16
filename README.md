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
int f_read(int , int ,void*);
int f_write(void *, int, int );
int f_opendir(char * );
dentry *f_readdir(int );
int f_mkdir(char * );
int f_rmdir(char * );
void init_library(char *);
void close_library();
```


We keep 3 global data structures shared between the shell and the
library. The first is the open fd table, where we keep track of 
open files and directories and the inodes of the files they refer to.
The second is the open inode table, where we keep the inodes of open 
files and directories in memory for faster read and write. The third 
is a character array holding the current working directory, pwd. This 
is mostly modified and used by the shell, but initialized by the lib.

The functions basic structure is as follows:
- f\_open takes a string with the name of the absolute path of the file to be opened, 
which is handled by the shell, and an int of 0 or 1 to specify CREATE or not, with 1 
meaning create. It returns a file descriptor which can be used most of the other 
library functions. -1 if the file doesn't exist. 
- f\_read takes a file descriptor, a number of bytes, and a buffer to copy to. It returns 
the number of bytes read to facilitate more.
- f\_write takes the same input as read except that it reads from the buffer. Same return 
mechanism.
- f\_opendir is the same as f\_open except that it sets the working directory, a global of 
the library that allows for relative paths on the library side.
_ f\_readdir takes an fd returned by opendir and returns 1 dentry of the directory referred to 
by the file descriptor. Consecutive calls to f\_readdir return consecutive dentries. After 
we run out of dentries in a directory, f\_readdir returns NULL. 
- f\_mkdir takes a name and craetes a dir for it in the current directory. Doesn't support 
absolute path. Returns 0 or -1.
- f\_rmdir takes a folder name and deletes it if it is empty. Returns 0 or -1.
- init\_library is used to initialize the globals. First thing the shell does.
- close\_library is used to free all library allocated memory and save the disk state.

## Implemented Features

## Tests

## Limitations
We chose to allow for the following limitations in order implement all functionality in the given time.

- We only read up to singly indirect blocks. Doubly and triply indirect blocks are not being 
read to save implementation time. Any file greater than 10\*512 + 128\*512 bytes will cause issues. 
- We only write small files, smaller than or equal to 5 kb (fits in direct blocks). Again this is 
just to save time to implement other functionality. HW6 had us navigate the entire i2node and i3node 
blocks so it makes sense to allocate the time for more interesting functionality.
- directories are only allowed 1 block of dentries. That means that a given directory, with an average 
file name length of 8 bytes and dentry size of 20, can hold only up to 18 files and directories. This 
makes readdir much simpler and less time-consuming.
- permissions are not implemented at all and error checking is minimal.
- .. is not included in directories. Going backwards in general is not implemented. This is ok 
given the limitations of the disk size anyways. However, you can always go back to root using cd with 
no arguments.
- Closing files and directories is not implemented (at least for now). It's as easy as removing the 
open fd table entry but it doesn't add any functionality to the shell so we chose to focus on other parts.
- Seek is not implemented (for now). Seek happens automatically with readdir but should be implemented in general. 
This is a matter of changing the seek position in the open fd table so it won't be a problem.
- none of ls flags are implemented, so is the f\_stat library function.
- rmdir is implemented. rm isn't for now.
- only > is implemented in redirection. It shows the functionality of f\_write.

## Memory Management
All memory is freed properly except one malloc at exit that is to be fixed.
