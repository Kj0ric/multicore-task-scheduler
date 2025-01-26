#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "constants.h"
#include "wbq.h"
#include <math.h>

extern int stop_threads;
extern int finished_jobs[NUM_CORES];
// IRRELEVANT extern int flag_array[NUM_CORES];
extern WorkBalancerQueue** processor_queues;

double calculateAvgQueueSize() {
    double total_size = 0;
    for (int i = 0; i < NUM_CORES; i++) {
        total_size += atomic_load(&processor_queues[i]->q_size);
    }
    return total_size / NUM_CORES;
}

double calculateAbsoluteDeviation(double avg_q_size) {
    double total_deviation = 0.0;
    for (int i = 0; i < NUM_CORES; i++) {
        int queue_size = atomic_load(&processor_queues[i]->q_size);
        total_deviation += fabs(queue_size - avg_q_size);
    }
    return total_deviation / NUM_CORES;
}

WorkBalancerQueue* findTaskWithMaxLoad(WorkBalancerQueue* my_queue, int my_id) {
    WorkBalancerQueue* max_load_queue = NULL;
    int max_size = -1;
    // Iterate through all queues
    for (int i = 0; i < NUM_CORES; i++) {
        if (i == my_id) continue; // Skip own queue

        int queue_size = atomic_load(&processor_queues[i]->q_size);
        if (queue_size > max_size) {
            max_size = queue_size;
            max_load_queue = processor_queues[i];
        }
    }
    return max_load_queue; // Return the queue with the maximum load
}

void updateHWandLW(double* hw, double* lw) {
    double avg_q_size = calculateAvgQueueSize();
    double abs_dev = calculateAbsoluteDeviation(avg_q_size);

    // Adjust thresholds dynamically
    *hw = avg_q_size + abs_dev;
    *lw = avg_q_size - abs_dev;
}

// Thread function for each core simulator thread
void* processJobs(void* arg) {
    // initalize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    WorkBalancerQueue* my_queue = my_arg -> q;
    int my_id = my_arg -> id;

    /* 
    * The bookkeeping of finished jobs is done in executeJob's example implementation.
    * After getting a task to execute the thread should call executeJob to simulate its execution. 
    It is okay if the thread does busy waiting when its queue is empty and no other job is available
    outside.
    */ 
    int iter_count = 0;
    double hw, lw;
    while (!stop_threads) {
        if (iter_count % 3 == 0) { // Update thresholds every n iterations
            updateHWandLW(&hw, &lw);
        }
        iter_count++;

        // Step 1: Monitor job load. Heavily loaded or under utilized?
        int queue_size = atomic_load(&my_queue->q_size); 

        // Step 2: Dynamic load balancing     
        // If underloaded, find the most loaded queue and attempt to steal
        if (queue_size < lw) {
            WorkBalancerQueue* max_queue = findTaskWithMaxLoad(my_queue, my_id);
            if (max_queue && atomic_load(&max_queue->q_size) > hw) {
                Task* taskFromOther = fetchTaskFromOthers(max_queue);
                if (taskFromOther) {
                    submitTask(my_queue, taskFromOther); // Add stolen task to own queue
                }
            }
        }
        
        // Step 3: Job execution
        Task* my_task = fetchTask(my_queue);  

        if (my_task){
            executeJob(my_task, my_queue, my_id);  

            // If my_task is unfinished requeue it
            if (my_task->task_duration > 0){
                submitTask(my_queue, my_task);
            }
            else{
                free(my_task); // Free finished task
                my_task = NULL;
            }
        }        
    }
    // If stop_threads is set to a non-zero value, terminate the procedure
}

// Do any initialization of your shared variables here.
// For example initialization of your queues, any data structures 
// you will use for synchronization etc.
void initSharedVariables() {
    // Initialize processor_queues array
    for (int i = 0; i < NUM_CORES; i++) {
        wbqInit(processor_queues[i]);  // Call your custom queue initialization function
    }

    // Initialize finished_jobs array
    for (int i = 0; i < NUM_CORES; i++) {
        finished_jobs[i] = 0;          // No jobs are finished at the start
    }

    // Initialize stop_threads to 0
    stop_threads = 0;                 
}

