# linux-filesystem

### Design of the file system

Thegr

#### Shell-site

### Design deviation

### What works and what doesn't

Library functions that we have implemented are:

```
int f_open(char * , int);
/* returns a file number of the open file */

int f_read(int fileno, int number_of_bytes, 
		void* pointer_to_read_buffer );

int f_write(void *, int, int );
int f_opendir(char * );
dentry *f_readdir(int );
int f_mkdir(char * );
int f_rmdir(char * );
void init_library(char *);
void close_library();
```

### Testing
