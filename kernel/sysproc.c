#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

struct xv6timer_t {
    int expiry;         // Number of ticks between interrupts
    uint next_tick;     // Tick count when timer should trigger next
    struct proc *proc;  // Associated process
    void (*callback)(struct xv6timer_t *);  // Callback function
};

void xv6timer_init(struct xv6timer_t *ptimer, struct proc *proc);
void xv6timer_forward(struct xv6timer_t *ptimer, int expiry);
void xv6timer_register_callback(struct xv6timer_t *ptimer, void (*callback)(struct xv6timer_t *));
void xv6timer_interrupt(struct xv6timer_t *ptimer);

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64 
sys_set_cpu_affinity(void) {
    int mask;
    argint(0, &mask);

    struct proc *p = myproc();
    acquire(&p->lock);
    p->cpu_mask = mask;
    release(&p->lock);
    
    return 0;  // Success
}

void period_callback(struct xv6timer_t *timer) {
    struct proc *p = timer->proc;
    acquire(&p->lock);
    if(p->state == SLEEPING) {
        p->state = RUNNABLE;
    }
    release(&p->lock);
}

//*NEW VERSION*/
/*
uint64 sys_setperiod(void) {
    int period;
    argint(0, &period);
    if (period < 1) {
        return -1;  // Invalid period
    }

    struct proc *p = myproc();
    if (p->timer == 0) {
        // Allocate a new timer for the process if it doesn't already have one
        p->timer = (struct xv6timer_t*)kalloc();
        if (p->timer == 0) {
            return -1;  // Memory allocation failed
        }

        xv6timer_init(p->timer, p);  // Initialize the timer
        p->period = period;  // Set the period
        p->next_run = ticks + period;  // Set the next run time
        xv6timer_register_callback(p->timer, period_callback);  // Register the callback
    }

    // Update the period and next run time
    p->period = period;
    p->next_run = ticks + period;
    
    return 0;
}

uint64 sys_wait_until_next_period(void) {
    struct proc *p = myproc();
    if (p->timer == 0) {
        return -1;  // Process is not periodic
    }

    acquire(&p->lock);  // Lock the process to modify its state

    // Sleep until the next period
    while (ticks < p->next_run) {
        p->state = SLEEPING;  // Set process state to sleeping
        release(&p->lock);    // Release lock before calling sched()
        sched();              // Yield the CPU
        acquire(&p->lock);    // Reacquire the lock after being scheduled
    }

    // Update the next run time for the periodic task
    p->next_run += p->period;

    release(&p->lock);  // Release the lock
    return 0;
}
*/

//OLD VERSION
///*
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
//*/