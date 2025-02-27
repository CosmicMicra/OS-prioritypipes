#include "kernel/types.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/spinlock.h"

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
