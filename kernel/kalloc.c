// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
} kmem;

int refcount[PHYSTOP / PGSIZE];

void kinit() {
    initlock(&kmem.lock, "kmem");
    freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end) {
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE) {
        refcount[(uint64)p / PGSIZE] = 1;
        kfree(p);
    }
}

// (+) 增加计数引用，用于 CoW
void incref(uint64 pa) {
    int pn = pa / PGSIZE;
    acquire(&kmem.lock);
    if (pa >= PHYSTOP || refcount[pn] < 1) {
        panic("incref");
    }
    refcount[pn]++;
    release(&kmem.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
    struct run *r;

    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP) {
        panic("kfree");
    }
    
    // (+) kfree 写时复制的页表，对于有引用的计数
    // 不要删除，而是减少引用
    acquire(&kmem.lock);
    int pn = (uint64)pa / PGSIZE;
    if (refcount[pn] < 1) {
        panic("kfree cow");
    }
    refcount[pn]--;
    release(&kmem.lock);
    
    if (refcount[pn] > 0) {
        return;
    }

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run *)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

// 申请一个 4096-byte 的物理页.
// 返回一个内核可以使用的指针.
// 如果内存无法申请，则返回 0.
void* kalloc(void) {
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
        int pn = (uint64)r / PGSIZE;
        if (refcount[pn] != 0) {
            panic("kalloc: ref");
        }
        refcount[pn] = 1;
    }
        
    release(&kmem.lock);

    if (r) {
        memset((char *)r, 5, PGSIZE); // fill with junk
    }
        
    return (void *)r;
}

// 计算系统的空闲内存，单位为 bytes.
int kmemamount(void) {
    int amount = 0;
    struct run* r;

    r = kmem.freelist;
    while (r) {
        amount += PGSIZE;
        r = r->next;
    }

    return amount;
} 