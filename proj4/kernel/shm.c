#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "kernel/defs.h"

// Define the shared memory location
#define SHM_VA (MAXVA - 16 * PGSIZE)

// Static buffer for shared memory
static char *shm_memory = 0;
static int shm_refs = 0;  // Number of processes using the shared memory

// System call implementation
uint64
sys_shmget(void)
{
  int key;
  uint64 size;
  struct proc *p = myproc();
  
  // Get arguments
  argint(0, &key);
  argaddr(1, (uint64*)&size);
  
  // We only support key=0 and size=4096 as per assignment
  if(key != 0 || size != 4096)
    return -1;
  
  // Allocate memory if this is the first call
  if(shm_memory == 0) {
    shm_memory = kalloc();
    if(shm_memory == 0)
      return -1;
    memset(shm_memory, 0, PGSIZE);
  }
  
  // Map the shared memory into the process's address space
  if(mappages(p->pagetable, SHM_VA, PGSIZE, (uint64)shm_memory, PTE_R|PTE_W|PTE_U) < 0)
    return -1;
  
  shm_refs++;
  return SHM_VA;
}

// Handle fork - map shared memory in child process
int
shmem_fork(pagetable_t old, pagetable_t new)
{
  // Check if parent has shared memory mapped
  pte_t *pte = walk(old, SHM_VA, 0);
  if(pte && (*pte & PTE_V)) {
    // Map it in the child
    if(mappages(new, SHM_VA, PGSIZE, (uint64)shm_memory, PTE_R|PTE_W|PTE_U) < 0)
      return -1;
    shm_refs++;
  }
  return 0;
}

// Clean up when process exits
void
shmem_exit(void)
{
  struct proc *p = myproc();
  
  // Check if process has shared memory mapped
  pte_t *pte = walk(p->pagetable, SHM_VA, 0);
  if(pte && (*pte & PTE_V)) {
    uvmunmap(p->pagetable, SHM_VA, 1, 0);  // Unmap but don't free
    shm_refs--;
    
    if(shm_refs == 0 && shm_memory) {
      kfree(shm_memory);
      shm_memory = 0;
    }
  }
}