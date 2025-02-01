#ifndef SPINLOCK_H
#define SPINLOCK_H

// Mutual exclusion lock.
struct spinlock {
  uint locked;       

  
  char *name;        
  struct cpu *cpu;   

  // Other members, such as a thread ID or a queue for waiting threads
};

// Function prototypes for spinlock operations
void init_spinlock(struct spinlock *lock);
void acquire(struct spinlock *lock);
void release(struct spinlock *lock);

#endif // SPINLOCK_H

