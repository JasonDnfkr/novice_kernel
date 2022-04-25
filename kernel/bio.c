// Buffer cache.
/*
    Buffer cache 缓冲区
    buffer cache 描述了 buf 结构的链表，保存磁盘内容的缓存
    以减少进程对磁盘的 I/O，为多个进程使用的磁盘提供同步机制（未完成）

    Interface:
    bread:   读取一个特定 disk block 的缓冲区
             返回一个锁住的 buffer
    bwrite:  将缓冲区内容写入磁盘
    brelse:  使用完缓冲区后，使用这个函数来处理锁
    * Do not use the buffer after calling brelse.
    * Only one process at a time can use a buffer,
         so do not keep them longer than necessary.
*/

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
    struct spinlock lock;
    struct buf buf[NBUF];

/*
    Linked list of all buffers, through prev / next.
    根据最近使用的频率排序（LRU 算法）
    head.next 是最频繁的，head.prev 是最少使用的
 */
    struct buf head;
} bcache;

void binit(void) {
    struct buf *b;

    initlock(&bcache.lock, "bcache");

    // 初始化 buffer cache 的节点
    // 首先是首尾相连
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        initsleeplock(&b->lock, "buffer");
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf *
bget(uint dev, uint blockno) {
    struct buf *b;

    acquire(&bcache.lock);

    // 检索这个 block 是否已经在 cache 中。遍历
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->dev == dev && b->blockno == blockno) {
            b->refcnt++;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    // 没有命中。
    // 根据 LRU 算法去除最不常用的缓存。
    for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if (b->refcnt == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0;
            b->refcnt = 1;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }
    panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *
bread(uint dev, uint blockno) {
    struct buf *b;

    b = bget(dev, blockno);
    if (!b->valid) {
        virtio_disk_rw(b, 0);
        b->valid = 1;
    }
    return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b) {
    if (!holdingsleep(&b->lock))
        panic("bwrite");
    virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
    if (!holdingsleep(&b->lock))
        panic("brelse");

    releasesleep(&b->lock);

    acquire(&bcache.lock);
    b->refcnt--;
    if (b->refcnt == 0) {
        // no one is waiting for it.
        b->next->prev = b->prev;
        b->prev->next = b->next;
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    release(&bcache.lock);
}

void bpin(struct buf *b) {
    acquire(&bcache.lock);
    b->refcnt++;
    release(&bcache.lock);
}

void bunpin(struct buf *b) {
    acquire(&bcache.lock);
    b->refcnt--;
    release(&bcache.lock);
}
