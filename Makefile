format: 
	cc format.c util.h -o format.o -I.
fs:
	cc fs.c util.h -o fs.o -I.

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
