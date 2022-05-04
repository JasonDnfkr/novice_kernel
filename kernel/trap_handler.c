#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

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