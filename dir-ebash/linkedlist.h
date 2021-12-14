#ifndef __LL_H
#define __LL_H

#include <sys/types.h>
#include <unistd.h>

#include "job.h"

struct Node  {
        jobtype * data;
        struct Node* next;
        struct Node* prev;
};

extern struct Node* head; // global variable - pointer to head node.

struct Node* GetNewNode(jobtype *);
void InsertAtHead(struct Node *);
void InsertAtTail(struct Node *);
void printnodesjob( struct Node * );
struct Node * findnodebyjobid( int  );
struct Node * findnodebypid( pid_t );
void printll();
void count();
void removenode(struct Node *);
struct Node* pop();
int getlen(char**);
void free2d(char**, int);
void freell();

#endif
