#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"

uint64 sys_setperiod(void) {
    int period;
    argint(0, &period);
    if(period < 1) return -1;
    
    struct proc *p = myproc();
    if(p->timer == 0) {
        p->timer = (struct xv6timer_t*)kalloc();
        if(p->timer == 0) return -1;
        
        
        xv6timer_init(p->timer, p);
        xv6timer_register_callback(p->timer, period_callback);
    }
    
    
    xv6timer_forward(p->timer, period);
    return 0;
}

uint64 sys_wait_until_next_period(void) {
    struct proc *p = myproc();
    if(p->timer == 0)
        return -1;
        
    acquire(&p->lock);
    p->state = SLEEPING;
    sched();
    release(&p->lock);
    return 0;
}