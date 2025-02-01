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
    struct pq_node* new_node = (struct pq_node*)kalloc(); // Allocate memory for the new node
    if (!new_node) return -1; // Return error if memory allocation fails

    new_node->task = *task;  // Copy task into the new node
    new_node->next = NULL;    // Initially, the next pointer is NULL

    acquire(pq->lock);  // Acquire lock for thread safety

    // Special case: if the queue is empty, just add the new node
    if (pq->head == NULL) {
        pq->head = new_node;
    } else {
        // Traverse the list to find the correct position based on task priority
        struct pq_node* current = pq->head;
        struct pq_node* prev = NULL;

        // Traverse through the list, finding the first task with lower priority
        while (current != NULL && current->task.priority >= task->priority) {
            prev = current;
            current = current->next;
        }

        // Insert the new node
        if (prev == NULL) {
            // Insert at the head of the list (highest priority)
            new_node->next = pq->head;
            pq->head = new_node;
        } else {
            // Insert between prev and current
            prev->next = new_node;
            new_node->next = current;
        }
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
