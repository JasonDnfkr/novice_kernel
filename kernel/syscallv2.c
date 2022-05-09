#include "include/types.h"
#include "include/riscv.h"
#include "include/defs.h"
#include "include/param.h"
#include "include/stat.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/fs.h"
#include "include/sleeplock.h"
#include "include/file.h"
#include "include/fcntl.h"
#include "include/sysinfo.h"


// 创建一个软连接。
uint64 sys_symlink() {
    char path[MAXPATH];
    char target[MAXPATH];

    if (argstr(0, target, MAXPATH) < 0 || argstr(1, path, MAXPATH) < 0) {
        return -1;
    }

    begin_op();

    struct inode *ip;
    if ((ip = create(path, T_SYMLINK, 0, 0)) == 0) {
        end_op();
        return -1;
    }

    // ilock(ip);

    if (writei(ip, 0, (uint64)target, 0, MAXPATH) < MAXPATH) {
        iunlockput(ip);
        end_op();
        return -1;
    }

    iunlockput(ip);
    end_op();
    
    return 0;
}

uint64 sys_strace(void) {
    int mask;
    if (argint(0, &mask) < 0) {
        return -1;
    }
    myproc()->mask = mask;
    return 0;
}

uint64 sys_sysinfo(void) {
    // printf("sss\n");
    struct sysinfo si;
    uint64 addr;
    if (argaddr(0, &addr) < 0) {
        return -1;
    }

    struct proc* p = myproc();
    si.freemem = kmemamount();
    si.nproc = procamount();

    // printf("si.freemem: %d, si.nproc: %d\n", si.freemem, si.nproc);

    if (copyout(p->pagetable, (uint64)addr, (char*)&si, sizeof(si))< 0) {
        return -1;
    }
    
    return 0;
}

uint64 sys_vmprint(void) {
    pgtblprint();
    return 0;
}

uint64 sys_sigalarm(void) {
    int interval;
    uint64 handler;
    if (argint(0, &interval) < 0) {
        return -1;
    }
    if (argaddr(1, &handler) < 0) {
        return -1;
    }
    if (interval == 0) {
        return 0;
    }
    
    struct proc* p = myproc();
    p->trapalarm.interval = interval;
    p->trapalarm.handler = (void*)handler;
    p->trapalarm.ticks = 0;
    
    return 0;
}

uint64 sys_sigreturn(void) {
    struct proc* p = myproc();
    *(p->trapframe) = p->trapalarm.trapframe_cp;
    memset(&p->trapalarm.trapframe_cp, 0, sizeof(struct trapframe));
    p->trapalarm.state = 0;
    return 0;
}