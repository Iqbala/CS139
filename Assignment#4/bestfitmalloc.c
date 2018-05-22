
/**
 * Assignment 4 - Malloc() and Free()
 * @author: Ali H. Iqbal
 **/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "bestfitmalloc.h"

static struct block *root;
static struct block *head = NULL;	


/**
 * Bestfit_malloc: A malloc() replacement called void *bestfit_malloc(int size) that allocates memory using 
 * the best-fit algorithm. Again, if no empty space is big enough, allocate more via sbrk(). A free() called void 
 * bestfit_free(void *ptr) that deallocates a pointer that was originally allocated by the malloc you wrote above.
 **/

void *bestfit_malloc(int size)
{
    if(head == NULL)
    {
        head = sbrk(sizeof(struct block));                                             
        root = head;
        void *ptr = sbrk(size);                                                             
        head->full = 1;
        //header size of whole block                                                                 
        head->size = size + sizeof(struct block);                                           
        head->prev = NULL;                                                                  
        head->next = NULL;
        return ptr;                                                                         
    }
    else
    {
        struct block *currLow;  
        struct block *currblock = head;                                             
        int low = -1; //to track block with low space                                                     

        while(currblock->next != NULL)                                                
        {
            if(currblock->full == 0 && currblock->size == (size + sizeof(struct block)))
            {
                currblock->full = 1;
                //return ptr to malloc start
                return ((char*)currblock + sizeof(struct block));                        
            }
            if(currblock->full == 0 && (currblock->size >= (size + sizeof(struct block))) && (low == -1 || currblock->size < low))
            {
                low = currblock->size;
                //store best fit                                              
                currLow = currblock;                                           
            }
            currblock = currblock->next;
        }
        //end of list
        if(low == -1)                                                                    
        {	//allocate block size to heap
            struct block *node = sbrk(sizeof(struct block));                               
            void *ptr = sbrk(size);                                                       
            node->full = 1;                                                                
            node->size = size + sizeof(struct block);    
            //set prev block last in list                                  
            node->prev = currblock;                                                    
            node->prev->next = node;                                                       
            node->next = NULL;
            //return ptr to malloc start
            return ptr;                                                                     
        }
        //if free spot is found > than needed
        if(low > sizeof(struct block) * 2 + size)                                        
        {		//create new node
            struct block *free = ( char *)currLow + size + sizeof(struct block);    

            free->size = currLow->size - size - sizeof(struct block);          
            //set new node free
            free->full = 0;                                                              
            free->next = currLow->next;                                           
            free->next->prev = free;
            free->prev = currLow;                                                 
            free->prev->next = free;

            //set size malloc + struct size
            currLow->size = size + sizeof(struct block);
            //next node = free                              
            currLow->next = free;                                                 
            currLow->next->prev = currLow;
            currLow->full = 1;
            return (currLow + sizeof(struct block));
        }
        //return lowest
        return currLow;
    }
}

/**
 * Bestfit_free: Your free function should coalesce adjacent free blocks. If the block that touches brk is free, 
 * you should use sbrk() with a negative offset to reduce the size of the heap
 **/

void bestfit_free(void *pt)
{
    if(pt - sbrk(0) > 0)
        return;

    struct block *nd, *prevNode = NULL, *nexNode = NULL;

    nd = pt - sizeof(struct block);

    if(nd->prev != NULL)
        prevNode = nd->prev;

    if(nd->next != NULL)
        nexNode = nd->next;

    if(prevNode == NULL && nexNode == NULL)
    {
        head = NULL;
        sbrk(-(nd->size));
        return;
    }

    //node free at tail
    else if(nexNode == NULL)                                                                       
    {
        int size = 0;

        if(prevNode->full == 0)
        {
            size = prevNode->size + nd->size;
            //if free and not head
            if(prevNode->prev != root)                                                              
            {
                prevNode = prevNode->prev;
                prevNode->next = NULL;
                sbrk(-(size));
                return;
            }
            else
            { //set all to null
                sbrk(-(size + root->size));                                   
                head = NULL;                                                                            
                return;
            }
        }
        else
        {
            prevNode->next = NULL;
            sbrk(-(nd->size));
            return;
        }
    }

    //free node at head
    else if(prevNode == NULL)                                                                      
    {
        int size = 0;

        if(nexNode->full == 0)                                                                        
        {
            size = nexNode->size + nd->size;
            //check if position is tail
            if(nexNode->next != NULL)                                                                
            {
                nexNode = nexNode->next;

                nd->size = size;
                nd->next = nexNode;
                //set new size                                                             
                nd->next->prev = nd;                                                                    
                nd->full = 0;
                return;
            }
            else
            { // free head to tail
                sbrk(-(size + root->size));                                       
                head = NULL;
                return;
            }
        }
        else                                                                                           
        {	//free node 
            nd->full = 0;
            return;
        }
    }
    //free node and coalesce with one behind
    else if(prevNode->full == 0)                                                                     
    {
        int newSize = 0;
        // if prev = 0, next = 0
        if(nexNode->full == 0)                                                                        
        { 	//newSize = all size combined
            newSize = prevNode->size + nd->size + nexNode->size;                                  

            if(nexNode->next != NULL && prevNode->prev != root)                                    
            {
                nexNode = nexNode->next;
                prevNode = prevNode->prev;
                nd->size = newSize;
                nd->prev = prevNode;
                nd->prev->next = nd;
                nd->next = nexNode;
                nd->next->prev = nd;
                nd->full = 0;
                return;
            }  
            else if(nexNode->next == NULL && prevNode->prev == root)                               
            {	//set nodes to null
                sbrk(-(newSize + root->size));                                                          
                head = NULL;
                return;
            }

            else if(nexNode->next != NULL && prevNode->prev == root)
            {
                nexNode = nexNode->next;

                nd->size = newSize;
                nd->prev = root;
                nd->next = nexNode;
                nd->next->prev = nd;
                nd->full = 0;
                return;
            }
            else if(nexNode->next == NULL && prevNode->prev != root)
            {
                prevNode = prevNode->prev;
                prevNode->next = NULL;
                sbrk(-(newSize));
                return;
            }

        }
        else                                                                              
        {
            newSize = prevNode->size + nd->size;

            if(prevNode->prev != root)                                                          
            {
                prevNode = prevNode->prev;
                nd->size = newSize;
                nd->prev = prevNode;
                nd->prev->next = nd;
                nd->full = 0;
                return;
            }
            else                                                                                        
            {
                nd->size = newSize;
                nd->prev = root;
                nd->full = 0;
                return;
            }
        }
    }
    //free from prev to tail
    else if(prevNode->full == 1)                                                                        
    {	//if prev = 1, next = 0 
        if(nexNode->full == 0) 
        {
            int newSize = nexNode->size + nd->size;

            if (nexNode->next != NULL)                                                                     
            {
                nd->size = newSize;
                nexNode = nexNode->next;
                nd->next = nexNode;
                nd->next->prev = nd;
                nd->full = 0;
                return;
            }
            else
            {
                prevNode->next = NULL;
                sbrk(-(newSize));                                                                      
                return;
            }
        }
         //only free within prev and next
        else
        {
            nd->full = 0;
            return;
        }

    }

}
