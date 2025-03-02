#include "../../kernel/types.h"
#include "../../kernel/fs.h"
#include "../../kernel/param.h"
#include "../../kernel/memlayout.h"
#include "../../kernel/riscv.h"
#include "../../kernel/defs.h"
#include "../../kernel/spinlock.h"
#include "../../kernel/proc.h"
#include "../../kernel/syscall.h"
#include "../../kernel/sleeplock.h"
#include "../../kernel/file.h"
#include "../../kernel/fcntl.h"

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
                prev_va = va;
                pages = 1;
            } else {
                pages++;
            }
        } else if (pages > 0) {
            printf("Region: 0x%lx - 0x%lx: %d\n", prev_va, va, pages);
            pages = 0;
        }
    }

    return 0;
}
