//
// Real-time pipe implementation with priority queue
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "pipe_rt.h"

static void
pq_insert(struct pq_node **head, struct task_t task)
{
    struct pq_node *new = kalloc();
    if (!new) return;

    new->task = task;
    new->next = 0;

    // Empty list or new task has higher priority
    if (*head == 0 || task.priority > (*head)->task.priority) {
        new->next = *head;
        *head = new;
        return;
    }

    // Find correct position
    struct pq_node *current = *head;
    while (current->next != 0 && 
           current->next->task.priority >= task.priority) {
        current = current->next;
    }

    new->next = current->next;
    current->next = new;
}

static struct task_t
pq_remove(struct pq_node **head)
{
    struct task_t task = {0};
    if (*head == 0) return task;

    struct pq_node *temp = *head;
    task = temp->task;
    *head = temp->next;
    kfree(temp);
    return task;
}

int
pipe_rt_alloc(struct file **f0, struct file **f1)
{
    struct pipe_rt *p;

    p = 0;
    *f0 = *f1 = 0;

    // Allocate pipe structure
    if ((p = kalloc()) == 0)
        goto bad;

    // Initialize pipe
    initlock(&p->lock, "pipe_rt");
    p->head = 0;
    p->nwrite = 1;
    p->nread = 1;
    p->active = 1;

    // Allocate file structures
    if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
        goto bad;

    // Set up read end
    (*f0)->type = FD_PIPE_RT;  // Changed from FD_PIPE
    (*f0)->readable = 1;
    (*f0)->writable = 0;
    (*f0)->pipe = (struct pipe*)p;

    // Set up write end
    (*f1)->type = FD_PIPE_RT;  // Changed from FD_PIPE
    (*f1)->readable = 0;
    (*f1)->writable = 1;
    (*f1)->pipe = (struct pipe*)p;

    return 0;

bad:
    if (p)
        kfree((char*)p);
    if (*f0)
        fileclose(*f0);
    if (*f1)
        fileclose(*f1);
    return -1;
}

void
pipe_rt_close(struct pipe_rt *p, int writable)
{
    acquire(&p->lock);
    if (writable) {
        p->nwrite--;
        wakeup(&p->nread);
    } else {
        p->nread--;
    }
    if (p->nwrite == 0 && p->nread == 0) {
        // Free all remaining tasks in queue
        while (p->head)
            pq_remove(&p->head);
        p->active = 0;
    }
    release(&p->lock);
}

int
pipe_rt_write(struct pipe_rt *p, uint64 addr, int n)
{
    struct proc *pr = myproc();
    struct task_t task;

    acquire(&p->lock);

    if (!p->active || p->nread <= 0) {
        release(&p->lock);
        return -1;
    }

    // Copy task from user space
    if (copyin(pr->pagetable, (char*)&task, addr, sizeof(task)) < 0) {
        release(&p->lock);
        return -1;
    }

    // Insert into priority queue
    pq_insert(&p->head, task);
    wakeup(&p->nread);  // Wake up readers
    release(&p->lock);

    return sizeof(task);
}

int
pipe_rt_read(struct pipe_rt *p, uint64 addr, int n)
{
    struct proc *pr = myproc();
    struct task_t task;

    acquire(&p->lock);

    while (p->head == 0 && p->active && p->nwrite > 0) {
        sleep(&p->nread, &p->lock);
    }

    if (p->head == 0) {
        release(&p->lock);
        return -1;
    }

    // Remove highest priority task
    task = pq_remove(&p->head);
    
    // Copy to user space
    if (copyout(pr->pagetable, addr, (char*)&task, sizeof(task)) < 0) {
        release(&p->lock);
        return -1;
    }

    release(&p->lock);
    return sizeof(task);
}

uint64
sys_pipe_rt(void)
{
    uint64 fdarray;
    struct file *rf, *wf;
    int fd0, fd1;
    struct proc *p = myproc();

    argaddr(0, &fdarray);

    if (pipe_rt_alloc(&rf, &wf) < 0)
        return -1;

    fd0 = -1;
    if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0) {
        if (fd0 >= 0)
            p->ofile[fd0] = 0;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }

    if (copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
        copyout(p->pagetable, fdarray+sizeof(fd0), (char*)&fd1, sizeof(fd1)) < 0) {
        p->ofile[fd0] = 0;
        p->ofile[fd1] = 0;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }

    return 0;
}