// init: 初始第一个用户进程

#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "kernel/include/spinlock.h"
#include "kernel/include/sleeplock.h"
#include "kernel/include/fs.h"
#include "kernel/include/file.h"
#include "user/user.h"
#include "kernel/include/fcntl.h"

char *argv[] = {"sh", 0};

int main(void) {
    int pid, wpid;

    if (open("console", O_RDWR) < 0) {
        mknod("console", CONSOLE, 0);
        open("console", O_RDWR);
    }
    dup(0); // stdout
    dup(0); // stderr

    while (1) {
        printf("[init] running shell\n");
        pid = fork();
        if (pid < 0) {
            printf("init: fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            exec("sh", argv);
            printf("init: exec sh failed\n");
            exit(1);
        }

        while (1) {
            // 调用 wait() 来接收 shell 进程的退出，
            // 但是 shell 不会退出，所以一直等
            wpid = wait((int *)0);
            if (wpid == pid) {
                // shell 挂了，重启 sh
                break;
            } else if (wpid < 0) {
                printf("init: wait returned an error\n");
                exit(1);
            } else {
                // init 没有父进程，持续循环.
            }
        }
    }
}
