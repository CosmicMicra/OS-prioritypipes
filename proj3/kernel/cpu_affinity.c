#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "kernel/syscall.h"

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