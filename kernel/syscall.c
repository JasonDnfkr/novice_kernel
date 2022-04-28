#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"


// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64 *ip) {
    struct proc *p = myproc();
    if (addr >= p->sz || addr + sizeof(uint64) > p->sz)
        return -1;
    if (copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
        return -1;
    return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char *buf, int max) {
    struct proc *p = myproc();
    int err = copyinstr(p->pagetable, buf, addr, max);
    if (err < 0)
        return err;
    return strlen(buf);
}

static uint64
argraw(int n) {
    struct proc *p = myproc();
    switch (n) {
    case 0:
        return p->trapframe->a0;
    case 1:
        return p->trapframe->a1;
    case 2:
        return p->trapframe->a2;
    case 3:
        return p->trapframe->a3;
    case 4:
        return p->trapframe->a4;
    case 5:
        return p->trapframe->a5;
    }
    panic("argraw");
    return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int *ip) {
    *ip = argraw(n);
    return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int argaddr(int n, uint64 *ip) {
    *ip = argraw(n);
    return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int argstr(int n, char *buf, int max) {
    uint64 addr;
    if (argaddr(n, &addr) < 0)
        return -1;
    return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);

extern uint64 sys_strace(void);
extern uint64 sys_symlink(void);

static uint64 (*syscalls[])(void) = {
    [SYS_fork]       sys_fork,
    [SYS_exit]       sys_exit,
    [SYS_wait]       sys_wait,
    [SYS_pipe]       sys_pipe,
    [SYS_read]       sys_read,
    [SYS_kill]       sys_kill,
    [SYS_exec]       sys_exec,
    [SYS_fstat]      sys_fstat,
    [SYS_chdir]      sys_chdir,
    [SYS_dup]        sys_dup,
    [SYS_getpid]     sys_getpid,
    [SYS_sbrk]       sys_sbrk,
    [SYS_sleep]      sys_sleep,
    [SYS_uptime]     sys_uptime,
    [SYS_open]       sys_open,
    [SYS_write]      sys_write,
    [SYS_mknod]      sys_mknod,
    [SYS_unlink]     sys_unlink,
    [SYS_link]       sys_link,
    [SYS_mkdir]      sys_mkdir,
    [SYS_close]      sys_close,

    [SYS_strace]     sys_strace,
    [SYS_symlink]    sys_symlink,
};

static char* syscalls_name[SYSNUM] = {
    "",
    "fork",
    "exit", 
    "wait",
    "pipe",
    "read", // 5
    "kill",
    "exec",
    "fstat",
    "chdir",
    "dup", // 10
    "getpid",
    "sbrk",
    "sleep",
    "uptime",
    "open", // 15
    "write",
    "mknod",
    "unlink",
    "link",
    "mkdir", // 20
    "close",
    "strace",
    // "sysinfo",
};

// static int syscalls_arg[SYSNUM] = {
//     0,
//     0,    // int    -fork(void);
//     1,    // int    -exit(int) __attribute__((noreturn));
//     1,    // int    -wait(int*);
//     1,    // int    -pipe(int*);
//     3,    // int  5 -read(int, void*, int);
//     1,    // int  6 -kill(int);
//     2,    // int  7 -exec(char*, char**);
//     2,    // int  8 -fstat(int fd, struct stat*);
//     1,    // int  9 -chdir(const char*);
//     1,    // int  10-dup(int);
//     0,    // int  11 getpid(void);
//     1,    // char*12-sbrk(int);
//     1,    // int  13-sleep(int);
//     0,    // int  14 uptime(void);
//     2,    // int  15-open(const char*, int);
//     3,    // int  16-write(int, const void*, int);
//     3,    // int  17 mknod(const char*, short, short);
//     1,    // int  18-unlink(const char*);
//     2,    // int  19-link(const char*, const char*);
//     1,    // int  20-mkdir(const char*);
//     1,    // int  21-close(int);

//     1,    // int  22-strace(int);
//     // 2,    // int  23 symlink(const char*, const char*);
// };

#define __SYSPRINT_ON__

void sysprint(int num) {
    struct proc* p = myproc();
    #ifdef __SYSPRINT_ON__
    #define arg0 p->trapframe->a0
    #define arg1 p->trapframe->a1
    #define arg2 p->trapframe->a2
    #define arg3 p->trapframe->a3
    #define arg4 p->trapframe->a4
    #define arg5 p->trapframe->a5
    #endif
    printf("(");

    switch (num) {
    // 1 arg0: integer
    case SYS_fork:
    case SYS_exit:
    case SYS_dup:
    case SYS_sbrk:
    case SYS_sleep:
    case SYS_close:
    case SYS_strace:
    case SYS_kill:
    printf("%d", arg0);
    break;

    case SYS_read:
    case SYS_write:
        printf("%d, 0x%x, %d", arg0, arg1, arg2);
        break;

    // 2 arg0: pointer   arg1: pointer
    case SYS_exec:
    case SYS_link:
    case SYS_symlink:
        printf("0x%x, 0x%x", arg0, arg1);
        break;

    // 1 arg0: pointer
    case SYS_wait:
    case SYS_pipe:
    case SYS_chdir:
    case SYS_unlink:
    case SYS_mkdir:
        printf("0x%x", arg0);
        break;

    // 2 arg0: integer   arg1: pointer
    case SYS_fstat:
        printf("%d, 0x%x", arg0, arg1);
        break;

    // 3 arg0: pointer   arg1: integer   arg2: integer
    case SYS_mknod:
        printf("0x%x, %d, %d", arg0, arg1, arg2);
        break;

    // 2 arg0: pointer   arg1: integer
    case SYS_open:
        printf("0x%x, %d", arg0, arg1);
        break;

    // no args
    default:
        break;
    }


    printf("): ");
}

#undef __SYSPRINT_ON__


void syscall(void) {
    int num;
    struct proc *p = myproc();

    num = p->trapframe->a7;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        // (+) mini strace 输出 syscall 名称
        if ((p->mask & (1 << num)) != 0) {
            printf("%d: syscall %s", p->pid, syscalls_name[num]);
            if (num == SYS_exit) {
                printf("\n");
            }
            else {
                sysprint(num);
            }
        }
        p->trapframe->a0 = syscalls[num]();

        // (+) mini strace 输出返回值
        if ((p->mask & (1 << num)) != 0) {
            printf(" -> %d\n", p->trapframe->a0);
        }


    } else {
        printf("%d %s: unknown sys call %d\n",
               p->pid, p->name, num);
        p->trapframe->a0 = -1;
    }
}
