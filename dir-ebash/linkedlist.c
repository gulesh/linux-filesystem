/* Doubly Linked List implementation */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "linkedlist.h"
#include "job.h"
#define MAX_LINE 1024
#define MAX_TOKEN 128

//Creates a new Node and returns pointer to it.

struct Node* head; // global variable - pointer to head node.

/* initialize node and allocate */
struct Node* GetNewNode(jobtype *job) {
	struct Node* newNode
		= (struct Node*)malloc(sizeof(struct Node));
	newNode->data = job;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

//Inserts a Node at head of doubly linked list
void InsertAtHead(struct Node *newNode) {
	if(head == NULL) {
		head = newNode;
		return;
	}
	head->prev = newNode;
	newNode->next = head;
	head = newNode;
}

//Inserts a Node at tail of Doubly linked list
void InsertAtTail(struct Node *newNode) {
	struct Node* temp = head;
	if(head == NULL) {
		head = newNode;
		return;
	}
	while(temp->next != NULL) temp = temp->next; // Go To last Node
	temp->next = newNode;
	newNode->prev = temp;
}

/* returns the last node */
struct Node* pop() {
	struct Node* temp = head;
	while(temp->next != NULL) temp = temp->next; // Go To last Node
	return temp;
}

/* print job attribute of a job for debugging */
void printnodesjob( struct Node *givennode ){
	if ( givennode == NULL ){
		printf( "Node is NULL \n");
	} else {
		printf("Node->data->jobid: %d\n", givennode->data->jobid);
	}
}

/* returns node with a given jobid by traversal */
struct Node * findnodebyjobid( int jobid ){
	struct Node* temp = head;
	while( temp != NULL ){
		if( temp->data->jobid == jobid ){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;

}

/* finds a job node with a given pid */
struct Node * findnodebypid( pid_t pid ){
	struct Node* temp = head;
	while( temp != NULL ){
		if( temp->data->pid == pid ){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;

}

//Prints all the elements in linked list in forward traversal order
void printll() {
	struct Node* temp = head;
	if (temp == NULL)
		printf("No jobs\n");
	while(temp != NULL) {
		char status[10] = "Running";
		if (temp->data->running == 0){
			strcpy(status, "Suspended");
		}
		char* builder = malloc(MAX_LINE*sizeof(char));
		int i = 0;
		while(temp->data->t_arr[i]!=NULL){
			strcat(builder, temp->data->t_arr[i]);
			strcat(builder, " ");
			i++;
		}
		printf("[%d]\t%s\t%s  PID: %d\n",temp->data->jobid, status, builder, temp->data->pid);
		free(builder);
		temp = temp->next;
	}
	printf("\n");
}

/* debugging traversal counter */
void count(){
	struct Node* temp = head;
	int c = 0;
 	while(temp != NULL){
 		c++;	
		temp = temp->next;
	}
	free(temp);
}

/* removes a job node from the list, frees memory */
void removenode(struct Node *node){
	if (head == NULL || node == NULL){
      		printf("Nothing to remove!\n");
      		return;
  	}
	if(head == node){
		head = node->next;
	}
	if(node->next != NULL){
		node->next->prev = node->prev;
	}
	if(node->prev != NULL){
		node->prev->next = node->next;
	}
	free2d(node->data->t_arr, MAX_TOKEN);
	free(node->data);
  	free(node);
	return;

}

/* calculates length of a sting array */
int getlen(char** buf){
        int i = 0;
        int c = 0;
        while(buf[i]!=NULL){
                c++;
                i++;
        }
        return c;
}

/* frees a string aray of specified string count */
void free2d(char** buf, int sz){
        for(int i = 0; i<sz; i++){
                free (buf[i]);
        }
        free(buf);
}

void freell() {
	struct Node* temp = head;
	if(temp == NULL) return; // empty list, exit
	// Going to last Node
	while(temp->next != NULL) {
		temp = temp->next;
	}
	// Traversing backward using prev pointer
	while(temp != NULL) {
		free2d(temp->data->t_arr, MAX_TOKEN);
        	free(temp->data);
		temp = temp->prev;
	}
	free(head);
}
