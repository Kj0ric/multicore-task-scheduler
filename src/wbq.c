#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "wbq.h"

/* 
* Enqueue a task to the tail end
* Only callible by the owner thread 
* Increment q_size 
*/
void submitTask(WorkBalancerQueue* q, Task* _task) {
    WBQNode* newNode = malloc(sizeof(WBQNode));
    assert(newNode != NULL); // Make sure malloc didn't fail
    newNode->task= _task;
    newNode->next = newNode->prev = NULL;

    pthread_mutex_lock(&q->tail_lock);
    // Link tail with newNode
    newNode->prev = q->tail;
    q->tail->next = newNode;            
    q->tail = newNode;                  // Update tail to the last node (newNode)
    atomic_fetch_add(&q->q_size, 1);
    pthread_mutex_unlock(&q->tail_lock);
}

/* 
* Dequeue a task from HEAD end 
* Can be only called by the owner thread
* When the WBQ is empty it returns NULL. 
* Decrement q_size 
*/
Task* fetchTask(WorkBalancerQueue* q) {
    pthread_mutex_lock(&q->head_lock);  // Acquire own head lock
    // If WBQ is logically empty
    if (q->head->next == NULL){
        pthread_mutex_unlock(&q->head_lock);
        return NULL;
    }

    WBQNode* dummy = q->head;           
    WBQNode* new_head = dummy->next;    // new_head points to new dummy node
    Task* fetchedTask = new_head->task;
    
    q->head = new_head;                 // Make extracted node new dummy by moving head ptr to it 
    q->head->prev = NULL;               // Update new dummy's prev
    atomic_fetch_sub(&q->q_size, 1);    // Decrement queue size
    pthread_mutex_unlock(&q->head_lock);
    
    free(dummy);                        
    dummy = NULL;                       
    return fetchedTask;
}

/* 
* Dequeue a task from TAIL end 
* This method will not be called by the owner thread
* When the WBQ is empty it returns NULL.
* Cache affinity and ownership of the task fetched is done in executeJobs => No need here
*/
Task* fetchTaskFromOthers(WorkBalancerQueue* q) {
    pthread_mutex_lock(&q->tail_lock);
    pthread_mutex_lock(&q->head_lock);
    // If the queue is logically empty
    if(q->head->next == NULL){
        pthread_mutex_unlock(&q->head_lock);
        pthread_mutex_unlock(&q->tail_lock);
        return NULL;
    }

    WBQNode* to_be_fetched= q->tail;
    Task* fetchedTask = to_be_fetched->task;
    q->tail = q->tail->prev;                // Update tail to its prev
    q->tail->next = NULL;                   // Update new tail's next to NULL

    atomic_fetch_sub(&q->q_size, 1);
    pthread_mutex_unlock(&q->head_lock);    
    pthread_mutex_unlock(&q->tail_lock);    
    
    free(to_be_fetched);                    // Free the old dummy
    to_be_fetched = NULL;                   // Nullify itspointer
    return fetchedTask;
}

void wbqInit(WorkBalancerQueue* q) {
    // Initialize head and tail
    WBQNode* dummy = malloc(sizeof(WBQNode));
    dummy->next = dummy->prev = NULL;
    q->head = q->tail = dummy;

    // Initialize locks 
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);

    // Initialize queue size
    atomic_init(&q->q_size, 0);
}
