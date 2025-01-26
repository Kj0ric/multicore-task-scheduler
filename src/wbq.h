#ifndef WBQ_H
#define WBQ_H

#include <stdatomic.h>
// Structs and methods for WorkBalancerQueue, you can use additional structs 
// and data structures ON TOP OF the ones provided here.

// **********************************************************
//extern volatile atomic_int dynamic_hw, dynamic_lw;

typedef struct WorkBalancerQueue WorkBalancerQueue;
typedef struct WBQNode WBQNode; 

typedef struct ThreadArguments {
    WorkBalancerQueue* q;
    int id;
} ThreadArguments;

typedef struct Task {
    char* task_id;
    int task_duration;
	double cache_warmed_up;
	WorkBalancerQueue* owner;   
} Task;

// TODO: You can modify this struct and add any fields you may need
struct WorkBalancerQueue {
	WBQNode* head;
	WBQNode* tail;
    pthread_mutex_t head_lock, tail_lock;
    atomic_int q_size;       // Number of tasks
};

typedef struct WBQNode {
    Task* task;             // Avoid copying Task object
    WBQNode* next;
    WBQNode* prev;
} WBQNode;

// **********************************************************
// WorkBalancerQueue API **********************************************************
void submitTask(WorkBalancerQueue* q, Task* _task);
Task* fetchTask(WorkBalancerQueue* q);
Task* fetchTaskFromOthers(WorkBalancerQueue* q);

// You can add more methods to Queue API
void wbqInit(WorkBalancerQueue* q); 
// **********************************************************


// Your simulator threads should call this function to simulate execution. 
// Don't change the function signature, you can use the provided implementation of 
// this function. We will use potentially different implementations while testing.
void executeJob(Task* task, WorkBalancerQueue* my_queue, int my_id );

void* processJobs(void* arg);
void initSharedVariables();
#endif