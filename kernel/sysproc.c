#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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

int sys_show_vm_areas(void) {
    struct proc *p = myproc(); // Get the current process
    pagetable_t pt = p->pagetable;
    uint64 va;
    uint64 prev_va = 0;
    int pages = 0;

    printf("[Memory-mapped areas for process %d:]\n", p->pid);

    for (va = 0; va < MAXVA; va += PGSIZE) {
        pte_t *pte = walk(pt, va, 0);
        if (pte && (*pte & PTE_V)) {
            if (pages == 0) {
                // Start of a new memory-mapped region
                prev_va = va;
                pages = 1;
            } else {
                // Continuation of the previous region
                pages++;
            }
        } else if (pages > 0) {
            // End of a region, print details
            printf("0x%lx - 0x%lx: %d\n", prev_va, va, pages);
            pages = 0;
        }
    }

    return 0;
}
