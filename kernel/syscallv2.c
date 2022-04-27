#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

// 从 sysfile.c 中复制而来
// static struct inode* 
// create(char *path, short type, short major, short minor) {
//     struct inode *ip, *dp;
//     char name[DIRSIZ];

//     if ((dp = nameiparent(path, name)) == 0)
//         return 0;

//     ilock(dp);

//     if ((ip = dirlookup(dp, name, 0)) != 0) {
//         iunlockput(dp);
//         ilock(ip);
//         if (type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
//             return ip;
//         iunlockput(ip);
//         return 0;
//     }

//     if ((ip = ialloc(dp->dev, type)) == 0)
//         panic("create: ialloc");

//     ilock(ip);
//     ip->major = major;
//     ip->minor = minor;
//     ip->nlink = 1;
//     iupdate(ip);

//     if (type == T_DIR) { // Create . and .. entries.
//         dp->nlink++;     // for ".."
//         iupdate(dp);
//         // No ip->nlink++ for ".": avoid cyclic ref count.
//         if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
//             panic("create dots");
//     }

//     if (dirlink(dp, name, ip->inum) < 0)
//         panic("create: dirlink");

//     iunlockput(dp);

//     return ip;
// }

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