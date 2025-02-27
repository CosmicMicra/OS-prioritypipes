#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "proj3/kernel/timer.h"

typedef void (*xv6timer_callback_t)(struct xv6timer_t *);
struct xv6timer_t;  
struct proc;  

void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc) {
    ptimer->proc = proc;
    ptimer->expiry = 0;
    ptimer->next_tick = 0;
    ptimer->callback = 0;
}

void xv6timer_forward(struct xv6timer_t *ptimer, int expiry) {
    ptimer->expiry = expiry;
    ptimer->next_tick = ticks + expiry;  
}

void xv6timer_register_callback(struct xv6timer_t *ptimer, void (*callback)(struct xv6timer_t *)) {
    ptimer->callback = callback;
}

void xv6timer_interrupt(struct xv6timer_t *ptimer) {
    if (ptimer->callback && ticks >= ptimer->next_tick) {
        ptimer->callback(ptimer);  
        ptimer->next_tick = ticks + ptimer->expiry;  
    }
}

void period_callback(struct xv6timer_t *timer) {
    struct proc *p = timer->proc;
    acquire(&p->lock);
    if(p->state == SLEEPING) {
        p->state = RUNNABLE;
    }
    release(&p->lock);
}