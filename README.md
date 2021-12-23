# HW7

### Gulesh, Ivan, and Mohamed

## Compilation

PLEASE REFER TO THE LIMITAIONS SECTION BEFORE COMPILING AND RUNNING THIS PROGRAM.

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
int f_read(int fd, int number_of_bytes, 
		void* pointer_to_read_buffer );
/* reads a number_of_bytes from a file specified into the buffer */

int f_open(char * fd, int read_write_mode);
/* returns a file number of the open file */

int f_write(void * buffer, int size, int  fd);
/* writes size bytes from buffer into file specified */

int f_opendir(char * path);
/* opens a directory specified by valid path */

dentry *f_readdir(int fd);
/* returns a dentry structure for a given directory specified
 * by filedescriptor */

int f_mkdir(char * name);
/* create a directory with a specified name in the current directory */

int f_rmdir(char * path);
/* deletes a directory specified by an absolute path */

void init_library(char * disk);
/* initialize a library with a given disk file */

void close_library();
/* close library */

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

Library features available are summarized under library function
signatures above.
Function's behaviour, including return values, mostly mimics that of 
standard Unix functions.
Deviations are listed under limitations section.

### Shell features

The core of the shell is taken from hw2.
The shell parses for special commands to do with the file system, and if
not found, proceeds to attempting execution.
Shell-site, we have a path parser implemented.
Relative paths are converted to absolute paths using a path global
library variable.
Note, that `.` is the current directory and can be referenced in
relative paths.

Special commands implemented are:
(note, that path means absolute or relative path).

- cd (path)
Takes an absolute or relative path as a $1, changes directory into it.
If no path is specified, changes to root (/).

- ls (path)
Lists files, folders, and mounting points in the path.
Please notice the coloring used to distinguish directories from files!
If no path is specified, current dir is listed.

- mkdir (name)
Creates a directory with a name inside current directory.

- pwd
Prints the absolute path of the current directory.

- cat path-to-file
Takes a path to file and prints it line by line until end of file.

- rmdir path
Deletes a directory specified by path.

- rm path-to-file
Deletes a file specified by path (unstable).

- more path-to-file
Takes a path to file and prints it 5 lines at a time.
Press enter to continue printing.

- redirection (>)
No appendage behaviour.
Can redirect from stdin to our file system as specified by posix.
For example: 
`less fs.h > fs.h` redirects the linux-fs file fs.h to fs.h on our
filesystem.
`echo e > e.txt` writes a file e.txt containing e.

- exit

## Tests

We created a disk using the format utility with a simple fs and a text
file down in it.
For each written command, we attempt to test corner cases: specifying
invalid paths, empty files, non-empty directories.



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
given the limitations of the disk size anyways. However, you can always go back to root using cd with no arguments.
- Closing files and directories is not implemented (at least for now). It's as easy as removing the open fd table entry but it doesn't add any functionality to the shell so we chose to focus on other parts.
- Seek is not implemented (for now). Seek happens automatically with readdir but should be implemented in general. 
This is a matter of changing the seek position in the open fd table so it won't be a problem.
- none of ls flags are implemented, so is the f\_stat library function.
- rmdir is implemented. rm isn't for now.
- only > is implemented in redirection. It shows the functionality of f\_write.
- format utility only creates 1 mb disk and doesn't have the option to create larger disks.
- persistence on disk has an issue where the first created dir dowsn't show (but still exist). In the new DISK. In fact, this issue sometimes interfere with files in nested directories.

## Memory Management

All memory is freed properly except one malloc at exit that is to be fixed.

