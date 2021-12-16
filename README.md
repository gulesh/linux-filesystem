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
library

## Implemented Features

Library features available are summarized under library function
signatures above.
Function's behaviour, including return values, mostly mimics that of 
standard Unix functions.
Deviations are listed under problematic features section.

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

## Problematic Features

## Memory Management

