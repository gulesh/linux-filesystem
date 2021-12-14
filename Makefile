format: 
	cc format.c util.h -o format.exec -I.
fs:
	cc fs.c util.h -o fs.exec -I.

path:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

lib: path
	gcc -Wall -fpic  -c fs.c util.h
	gcc -o libfs.so fs.o -shared
	gcc -o mysh mysh.c -L. -lfs


b: mysh.c fs.c util.h
	cc fs.h mysh.c  util.h -o bb -I.

ebash:
	gcc -c -o linkedlist.o linkedlist.c -I.
	gcc -c -o job.o job.c -I.             
	gcc -c -o ebash.o ebash.c -I.        
	gcc -o ebash linkedlist.o job.o ebash.o -I.
	chmod 770 ebash 

mysh: mysh.c
	cc mysh.c -o mysh

clean:
	rm *.o
