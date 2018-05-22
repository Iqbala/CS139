/*
*  Sample Driver Program to test Project 4
*
*  Replacing the bestfit_malloc and bestfit_free 
*  with malloc and free should give you the desired
*  outputs for the program
*/

/**
 * Assignment 4 - Implementation of Malloc() and Free()
 * @author: Ali H. Iqbal
 **/

#include <stdio.h>
#include <string.h>

// Include your header file for bestfit_malloc and bestfit_free
#include "bestfitmalloc.h"

// bestfit_malloc
#define MALLOC bestfit_malloc
// bestfit_free
#define FREE bestfit_free


// Struct for testing a linked list
struct node{
int key;
int value;
char name[10];
struct node* next;
} *head;


void test_linked_list(void);
void test_int_pointers(void);

int main(){

	char *str1 = "Hello World\0";
	char *str2, *str3;
	
	// Replace with your function for malloc call
	str2 = (char*) MALLOC(strlen(str1)+1);
	strcpy(str2,str1);
	printf("\n%s", str2);

	FREE(str2);

	test_linked_list();
	test_linked_list();

    	test_int_pointers();
    
	return;
}

// Function to test integer pointer allocation 
void test_int_pointers(void){
     
	int *ptr;
	ptr = (int *)MALLOC(sizeof(int));
	*ptr = 999;
	printf("\n*ptr is: %d\n\n",*ptr);	
	FREE(ptr);
     
}

// Function to test linked list
void test_linked_list(void){

	// Replace with your function for malloc
	head = (struct node*)MALLOC(sizeof (struct node));
	head->next = NULL;
	head->key = 100;

	// Replace with your function for malloc
	head->next = (struct node*)MALLOC(sizeof (struct node));
	head->next->key = 200;
	head->next->next = NULL;

	// Replace with your function for malloc
	head->next->next = (struct node*)MALLOC(sizeof (struct node));
	head->next->next->key = 200;
	head->next->next->next = NULL;
	
    	printf("\nHead->key is: %d", head->key);
	printf("\nHead->next->key is: %d\n", head->next->key);
	
	printf("\nSize of my struct: %d\n\n", sizeof(struct node));
	
	struct node *temp1;
	temp1 = head->next->next;
	head->next->next = NULL;

	struct node *temp2;
	temp2 = head->next;
	head->next = NULL;

	// Relplace with your function for free
	FREE(temp2);
}
