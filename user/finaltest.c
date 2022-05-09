#include "kernel/include/param.h"
#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"
#include "kernel/include/fs.h"
#include "kernel/include/fcntl.h"
#include "kernel/include/syscall.h"
#include "kernel/include/memlayout.h"
#include "kernel/include/riscv.h"

// 测试内核是否允许访问超限的地址
void MAXVAplus(char* s) {
    volatile uint64 a = MAXVA;
    printf("最大地址：%p\n", MAXVA);
    for (; a != 0; a <<= 1) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            printf("%s: fork 失败\n", s);
            exit(1);
        }
        else if (pid == 0) {
            printf("尝试访问地址：%p\n", a);
            *(char *)a = 99;
            printf("%s: [ERROR]%x 被写入.", s, a);
            exit(1);
        }
        else {
            int xstatus;
            wait(&xstatus);
            if (xstatus != -1) {
                exit(1);
            }
        }
    }
}

// 测试文件系统新建文件
void createfile(char* s) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("%s: fork 失败\n", s);
        exit(1);
    }
    else if (pid == 0) {
        char filename[10] = "file000";
        for (int i = 0; i < 30; i++) {
            char oneth = i % 10 + '0';
            char twoth = i / 10 + '0';
            filename[5] = twoth;
            filename[6] = oneth;
            int fd = open(filename, O_CREATE);
            // printf("filename: %s\n", filename);
            if (fd < 0) {
                exit(-1);
            }
            close(fd);
        }
        
        pid_t pidexec = fork();
        if (pidexec < 0) {
            printf("%s: fork pidexec 失败\n", s);
            exit(1);
        }
        else if (pidexec == 0) {
            char* argv[] = {
                "ls", 0
            };
            printf("即将调用 ls 展示生成的文件...\n");
            sleep(10);
            exec("ls", (char**)argv);
        }
        else {
            int xstatus;
            wait(&xstatus);
            if (xstatus != 0) {
                exit(1);
            }
        }
    }
    else {
        int xstatus;
        wait(&xstatus);
        if (xstatus != 0) {
            exit(1);
        }        
    }
}


// 运行测试的函数
int run(void f(char *), char* s) {
    pid_t pid;
    int xstatus;

    printf("测试: %s\n", s);
    pid = fork();
    if (pid < 0) {
        printf("run: fork 失败\n");
        exit(1);
    }
    else if (pid == 0) {
        f(s);
        exit(0);
    }
    else {
        wait(&xstatus);
        if (xstatus != 0) {
            printf("测试 %s 失败.\n", s);
        }
        else {
            printf("测试 %s 成功.\n", s);
        }
        return xstatus;
    }
}

int main(int argc, char* argv[]) {
    printf("开始内核功能测试\n");

    struct test {
        void (*f)(char *);
        char* s;
    } tests[] = {
        { MAXVAplus, "访问超出内存限制的地址" },
        { createfile, "文件读写" },
        { 0, 0 },
    };

    for (struct test *t = tests; t->s != 0; t++) {
        run(t->f, t->s);
    }


    exit(0);
}