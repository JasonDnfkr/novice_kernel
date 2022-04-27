#define NPROC        64  // 最大支持: 进程数量
#define NCPU          8  // 最大支持: CPU core 数量
#define NOFILE       16  // 最大支持: 每个进程允许打开的最大打开文件数
#define NFILE       100  // 最大支持: OS 打开的文件数
#define NINODE       50  // 最大支持: active 的 inode 数量
#define NDEV         10  // 最大支持: major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // 最大支持: exec 系统调用的参数
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       200000  // 文件系统的允许写入的 disk block 数量
#define MAXPATH      128   // 最长文件名

#define MAXDEPTH     10  // symbolic link 最大递归深度