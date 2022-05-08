#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/defs.h"

void alarm_handler(void) {
    struct proc *p = myproc();
    if (p->trapalarm.interval != 0) { // 不为0，说明开启了 alarm 机制
        if (p->trapalarm.state == 0) {
            p->trapalarm.ticks++;
            if (p->trapalarm.ticks == p->trapalarm.interval) {
                p->trapalarm.ticks = 0;
                p->trapalarm.state = 1;
                p->trapalarm.trapframe_cp = *(p->trapframe);

                p->trapframe->epc = (uint64)p->trapalarm.handler;
            }
        }
    }
}

int pagefault_handler(void) {
    struct proc* p = myproc();

    uint64 va = r_stval();
    uint64 pa = (uint64)kalloc();
    if (pa == 0) {
        return -1;
    }
    else if (va >= p->sz || va < PGROUNDDOWN(p->trapframe->sp)) {
        kfree((void*)pa);
        return -2;
    }
    else {
        memset((void*)pa, 0, PGSIZE);
        va = PGROUNDDOWN(va);
        if (mappages(p->pagetable, va, PGSIZE, pa, PTE_R | PTE_W | PTE_X | PTE_U) != 0) {
            kfree((void*)pa);
            return -1;
        }
    }
    return 0;
}