#include "pqueue.h"
#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "param.h"
#include "proc.h"
#include <stddef.h>

#ifndef NULL
#define NULL ((void*)0)  // Manually define NULL if not already defined
#endif

// Initialize the priority queue
void pqueue_init(struct pqueue* pq) {
    pq->head = NULL;           // Initially, the queue is empty
    pq->lock = 0;              // No lock initially
}

// Push a task into the priority queue
int pqueue_push(struct pqueue* pq, struct task_t* task) {
    struct pq_node* new_node = (struct pq_node*)kalloc(); 

    new_node->task = *task; 
    new_node->next = NULL;    

    acquire(pq->lock);  

    // if the queue is empty,  add the new node
    if (pq->head == NULL|| task->priority > pq->head->task.priority) {
        new_node->next = pq->head;
        pq->head = new_node;
    } else {
        // find the correct position based on priority
        struct pq_node* current = pq->head;
        
        while (current->next && current->next->task.priority >= task->priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }

    

    release(pq->lock);  // Release lock after modification
    return 0;  // Successfully pushed the task into the queue
}

// Pop the highest-priority task from the queue
struct task_t* pqueue_pop(struct pqueue* pq) {
    acquire(pq->lock);  // Acquire lock for thread safety

    if (pq->head == NULL) {
        release(pq->lock);  // Release lock if the queue is empty
        return NULL;  // Queue is empty, return NULL
    }

    // Remove the head node (highest-priority task)
    struct pq_node* node_to_pop = pq->head;
    pq->head = pq->head->next;

    struct task_t* task = &node_to_pop->task;
    kfree(node_to_pop);  // Free the memory allocated for the node

    release(pq->lock);  // Release lock after modification
    return task;  // Return the task
}

// Check if the queue is empty
int pqueue_empty(struct pqueue* pq) {
    acquire(pq->lock);  

    int empty = pq->head == NULL;

    release(pq->lock);  
    return empty;  // Return if the queue is empty (1) or not (0)
}
