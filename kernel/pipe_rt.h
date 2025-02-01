#ifndef PIPE_RT_H
#define PIPE_RT_H

#include "types.h"
#include "spinlock.h"
#include "pqueue.h"



struct spinlock;  // Forward declaration
struct pq_node;   // Forward declaration

struct pipe_rt {
    struct spinlock lock;
    struct pq_node *head;
    int nwrite;
    int nread;
    int active;
};

int pipe_rt_write(struct pipe_rt *p, uint64 addr, int n);
int pipe_rt_read(struct pipe_rt *p, uint64 addr, int n);
int pipe_rt_alloc(struct file **f0, struct file **f1);
uint64 sys_pipe_rt(void);






#endif





