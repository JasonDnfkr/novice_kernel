// 内核切换时保存的 context
struct context {
    uint64 ra;
    uint64 sp;

    // callee-saved
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

// CPU 状态
struct cpu {
    struct proc *proc;      // 正在 CPU 上运行的进程，没有时为 null
    struct context context; // 
    int noff;               // Depth of push_off() nesting.
    int intena;             // 在 push_off() 之前，中断是否启用了?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
    /*   0 */ uint64 kernel_satp;   // 内核页表
    /*   8 */ uint64 kernel_sp;     // 内核进程的栈
    /*  16 */ uint64 kernel_trap;   // usertrap() 地址
    /*  24 */ uint64 epc;           // user program counter 位置
    /*  32 */ uint64 kernel_hartid; // saved kernel tp
    /*  40 */ uint64 ra;
    /*  48 */ uint64 sp;
    /*  56 */ uint64 gp;
    /*  64 */ uint64 tp;
    /*  72 */ uint64 t0;
    /*  80 */ uint64 t1;
    /*  88 */ uint64 t2;
    /*  96 */ uint64 s0;
    /* 104 */ uint64 s1;
    /* 112 */ uint64 a0;
    /* 120 */ uint64 a1;
    /* 128 */ uint64 a2;
    /* 136 */ uint64 a3;
    /* 144 */ uint64 a4;
    /* 152 */ uint64 a5;
    /* 160 */ uint64 a6;
    /* 168 */ uint64 a7;
    /* 176 */ uint64 s2;
    /* 184 */ uint64 s3;
    /* 192 */ uint64 s4;
    /* 200 */ uint64 s5;
    /* 208 */ uint64 s6;
    /* 216 */ uint64 s7;
    /* 224 */ uint64 s8;
    /* 232 */ uint64 s9;
    /* 240 */ uint64 s10;
    /* 248 */ uint64 s11;
    /* 256 */ uint64 t3;
    /* 264 */ uint64 t4;
    /* 272 */ uint64 t5;
    /* 280 */ uint64 t6;
};

enum procstate { UNUSED,
                 USED,
                 SLEEPING,
                 RUNNABLE,
                 RUNNING,
                 ZOMBIE };

// Per-process state
struct proc {
    struct spinlock lock;

    // 在使用以下内容时，p->lock 必须为 1（保持上锁）:
    enum procstate state; // 进程状态
    void *chan;           // 如果为 1，正在 sleep，chan 值
    int killed;           // 如果为 1，标记为 killed
    int xstate;           // Exit status to be returned to parent's wait
    int pid;              // 进程 ID

    // 在使用以下内容时，wait_lock 必须持有:
    struct proc *parent; // 父进程

    // 以下内容不需要持有 p->lock，因为对进程不可见。
    uint64 kstack;               // 内核栈的虚拟地址
    uint64 sz;                   // 进程占用内存 (bytes)
    pagetable_t pagetable;       // 用户页表
    struct trapframe *trapframe; // 存储 trampoline.S 代码的位置
    struct context context;      // 进程上下文。swtch() here to run process
    struct file *ofile[NOFILE];  // 打开的文件
    struct inode *cwd;           // 当前 dir
    char name[16];               // 进程名字 (debug用)

    int mask;                    // 给 strace 提供的变量
};
