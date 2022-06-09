#include "kernel/include/param.h"
#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"
#include "kernel/include/fs.h"
#include "kernel/include/fcntl.h"
#include "kernel/include/syscall.h"
#include "kernel/include/memlayout.h"
#include "kernel/include/riscv.h"

typedef uint pid_t;

#define BUFSZ ((MAXOPBLOCKS + 2) * BSIZE)

char buf[BUFSZ];

void forkfailed(char*);

// 测试内核是否允许访问超限的地址
void MAXVAplus(char* s) {
    volatile uint64 a = MAXVA;
    printf("最大地址：%p\n", MAXVA);
    for (; a != 0; a <<= 1) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            forkfailed(s);
        }
        else if (pid == 0) {
            printf("尝试访问地址：%p\n", a);
            *(char *)a = 99;
            printf("%s: [ERROR] %x 被写入.", s, a);
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

// 测试内存无限增长，最后应该要被 kill
void memalloc(char* s) {
    printf("测试用户进程无限增长，正常情况下应该被内核强制杀死\n");

    pid_t pid = fork();
    if (pid < 0) {
        forkfailed(s);
    }
    else if (pid == 0) {
        char* ptr = 0;
        int cnt = 0;
        while (1) {
            cnt++;
            ptr = (char*)malloc(4096);
            if (cnt % 1000 == 0) {
                printf("生成了 %d 个 page.\n", cnt);
            }
            *(ptr + 3) = '9';
        }
    }
    else {
        int xstatus;
        wait(&xstatus);
        if (xstatus != 0) { // 程序应该被中断
            exit(0);
        }
        exit(1);
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
            printf("即将调用 ls 展示生成的文件...\n");
            sleep(10);
            exec("ls", (char*[]){ "ls", 0 });
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

// 读内核地址
void kernmem(char *s) {
    char *a;
    int pid;

    for (a = (char *)(KERNBASE); a < (char *)(KERNBASE + 2000000); a += 50000) {
        pid = fork();
        if (pid < 0) {
            printf("%s: fork failed\n", s);
            exit(1);
        }
        if (pid == 0) {
            printf("访问内核地址 0x%x\n", a);
            printf("%s: oops could read %x = %x\n", s, a, *a);
            exit(1);
        }
        int xstatus;
        wait(&xstatus);
        if (xstatus != -1) // did kernel kill child?
            exit(1);
    }
}

// 测试 bss 段是否为 0
char uninit[10000];
void bsstest(char *s) {
    int i;

    for (i = 0; i < sizeof(uninit); i++) {
        if (uninit[i] != '\0') {
            printf("%s: bss test failed\n", s);
            exit(1);
        }
    }
}

// STACK OVERFLOW!!!
void stacktest(char *s) {
    int pid;
    int xstatus;

    pid = fork();
    if (pid == 0) {
        char *sp = (char *)r_sp();
        sp -= PGSIZE;
        printf("当前 sp = 0x%x\n", sp);
        // the *sp should cause a trap.
        printf("%s: stacktest: read below stack %p\n", s, *sp);
        exit(1);
    } else if (pid < 0) {
        printf("%s: fork failed\n", s);
        exit(1);
    }
    wait(&xstatus);
    if (xstatus == -1) // kernel killed child?
        exit(0);
    else
        exit(xstatus);
}


// fork 测试
void forktest(char *s) {
    enum { N = 1000 };
    int n, pid;
    printf("即将进行多次 fork(), 正常情况下将会输出字母\n");
    printf("fork 的数目取决于进程最大值\n");

    for (n = 0; n < N; n++) {
        pid = fork();
        if (pid < 0) {
            break;
        }
        if (pid == 0) {
            printf("%c", (char)(getpid() % 26  + 'A'));
            exit(0);
        } 
    }

    if (n == 0) {
        printf("%s: no fork at all!\n", s);
        exit(1);
    }

    if (n == N) {
        printf("%s: fork claimed to work 1000 times!\n", s);
        exit(1);
    }

    for (; n > 0; n--) {
        if (wait(0) < 0) {
            printf("%s: wait stopped early\n", s);
            exit(1);
        }
    }

    if (wait(0) != -1) {
        printf("%s: wait got too many\n", s);
        exit(1);
    }
    printf("\n");
}

// 简单地测试 open 和 close
void simplefs(char* s) {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        forkfailed(s);
    }
    else if (pid == 0) {
        int fd = open("file000", O_RDONLY);
        if (fd < 0) {
            printf("%s: open file000 failed!\n", s);
            exit(1);
        }
        close(fd);
        fd = open("doesnotexist", O_RDONLY);
        if (fd >= 0) {
            printf("%s: open doesnotexist succeeded!\n", s);
            exit(1);
        }

        enum { N = 100,
            SZ = 10 };

        fd = open("small", O_CREATE | O_RDWR);
        if (fd < 0) {
            printf("%s: [ERROR] creat small failed!\n", s);
            exit(1);
        }
        for (int i = 0; i < N; i++) {
            if (write(fd, "aaaaaaaaaa", SZ) != SZ) {
                printf("%s: [ERROR] write aa %d new file failed\n", s, i);
                exit(1);
            }
            if (write(fd, "bbbbbbbbbb", SZ) != SZ) {
                printf("%s: [ERROR] write bb %d new file failed\n", s, i);
                exit(1);
            }
        }
        close(fd);
        fd = open("small", O_RDONLY);
        if (fd < 0) {
            printf("%s: [ERROR] open small failed!\n", s);
            exit(1);
        }
        int i = read(fd, buf, N * SZ * 2);
        if (i != N * SZ * 2) {
            printf("%s: read failed\n", s);
            exit(1);
        }
        close(fd);

        printf("创建了文件 small 并写入了一些内容，将执行 cat small 查看...\n");
        sleep(15);
        pid_t pid2 = fork();
        if (pid2 < 0) {
            forkfailed(s);
        }
        else if (pid2 == 0) {
            exec("cat", (char*[]){ "cat", "small", 0 });
        }
        else {
            int xstatus;
            wait(&xstatus);
            if (xstatus != 0) {
                exit(1);
            }
        }

        if (unlink("small") < 0) {
            printf("%s: unlink small failed\n", s);
            exit(1);
        }

        exit(0);
    }
    else {
        int xstatus;
        wait(&xstatus);
        if (xstatus != 0) {
            exit(1);
        }
    }
}

void subdir(char *s) {
    int fd, cc;

    unlink("ff");
    if (mkdir("dd") != 0) {
        printf("%s: mkdir dd failed\n", s);
        exit(1);
    }
    printf("创建文件夹 /dd 成功.\n");

    fd = open("dd/ff", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("%s: create dd/ff failed\n", s);
        exit(1);
    }
    write(fd, "ff", 2);
    close(fd);
    printf("创建并写入文件 /dd/ff 成功.\n");

    if (unlink("dd") >= 0) {
        printf("%s: unlink dd (non-empty dir) succeeded!\n", s);
        exit(1);
    }

    if (mkdir("/dd/dd") != 0) {
        printf("subdir mkdir dd/dd failed\n", s);
        exit(1);
    }
    printf("创建文件夹 /dd/dd 成功.\n");


    fd = open("dd/dd/ff", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("%s: create dd/dd/ff failed\n", s);
        exit(1);
    }
    write(fd, "FF", 2);
    close(fd);
    printf("创建并写入文件 /dd/dd/ff 成功.\n");

    fd = open("dd/dd/../ff", 0);
    if (fd < 0) {
        printf("%s: open dd/dd/../ff failed\n", s);
        exit(1);
    }
    printf("打开文件 /dd/dd/../ff 成功.\n");
    cc = read(fd, buf, sizeof(buf));
    if (cc != 2 || buf[0] != 'f') {
        printf("%s: dd/dd/../ff wrong content\n", s);
        exit(1);
    }
    close(fd);
    printf("读取文件 /dd/dd/../ff 成功.\n");

    if (link("dd/dd/ff", "dd/dd/ffff") != 0) {
        printf("link dd/dd/ff dd/dd/ffff failed\n", s);
        exit(1);
    }

    if (unlink("dd/dd/ff") != 0) {
        printf("%s: unlink dd/dd/ff failed\n", s);
        exit(1);
    }
    if (open("dd/dd/ff", O_RDONLY) >= 0) {
        printf("%s: open (unlinked) dd/dd/ff succeeded\n", s);
        exit(1);
    }
    printf("读取文件(O_RDONLY) /dd/dd/ff 成功.\n");


    if (chdir("dd") != 0) {
        printf("%s: chdir dd failed\n", s);
        exit(1);
    }
    printf("切换到目录 /dd 成功.\n");

    if (chdir("dd/../../dd") != 0) {
        printf("%s: chdir dd/../../dd failed\n", s);
        exit(1);
    }
    printf("切换到目录 /dd/../../dd 成功.\n");

    if (chdir("dd/../../../dd") != 0) {
        printf("chdir dd/../../dd failed\n", s);
        exit(1);
    }
    printf("切换到目录 /dd/../../../dd 成功.\n");

    if (chdir("./..") != 0) {
        printf("%s: chdir ./.. failed\n", s);
        exit(1);
    }
    printf("切换到目录 ./.. 成功.\n");

    fd = open("dd/dd/ffff", 0);
    if (fd < 0) {
        printf("%s: open dd/dd/ffff failed\n", s);
        exit(1);
    }
    printf("打开文件 ./.. 成功.\n");

    if (read(fd, buf, sizeof(buf)) != 2) {
        printf("%s: read dd/dd/ffff wrong len\n", s);
        exit(1);
    }
    close(fd);

    if (open("dd/dd/ff", O_RDONLY) >= 0) {
        printf("%s: open (unlinked) dd/dd/ff succeeded!\n", s);
        exit(1);
    }

    if (open("dd/ff/ff", O_CREATE | O_RDWR) >= 0) {
        printf("%s: create dd/ff/ff succeeded!\n", s);
        exit(1);
    }


    if (open("dd/xx/ff", O_CREATE | O_RDWR) >= 0) {
        printf("%s: create dd/xx/ff succeeded!\n", s);
        exit(1);
    }

    if (open("dd", O_CREATE) >= 0) {
        printf("%s: create dd succeeded!\n", s);
        exit(1);
    }

    if (open("dd", O_RDWR) >= 0) {
        printf("%s: open dd rdwr succeeded!\n", s);
        exit(1);
    }

    if (open("dd", O_WRONLY) >= 0) {
        printf("%s: open dd wronly succeeded!\n", s);
        exit(1);
    }
    if (link("dd/ff/ff", "dd/dd/xx") == 0) {
        printf("%s: link dd/ff/ff dd/dd/xx succeeded!\n", s);
        exit(1);
    }
    if (link("dd/xx/ff", "dd/dd/xx") == 0) {
        printf("%s: link dd/xx/ff dd/dd/xx succeeded!\n", s);
        exit(1);
    }
    if (link("dd/ff", "dd/dd/ffff") == 0) {
        printf("%s: link dd/ff dd/dd/ffff succeeded!\n", s);
        exit(1);
    }
    if (mkdir("dd/ff/ff") == 0) {
        printf("%s: mkdir dd/ff/ff succeeded!\n", s);
        exit(1);
    }
    if (mkdir("dd/xx/ff") == 0) {
        printf("%s: mkdir dd/xx/ff succeeded!\n", s);
        exit(1);
    }
    if (mkdir("dd/dd/ffff") == 0) {
        printf("%s: mkdir dd/dd/ffff succeeded!\n", s);
        exit(1);
    }
    if (unlink("dd/xx/ff") == 0) {
        printf("%s: unlink dd/xx/ff succeeded!\n", s);
        exit(1);
    }
    if (unlink("dd/ff/ff") == 0) {
        printf("%s: unlink dd/ff/ff succeeded!\n", s);
        exit(1);
    }
    if (chdir("dd/ff") == 0) {
        printf("%s: chdir dd/ff succeeded!\n", s);
        exit(1);
    }
    if (chdir("dd/xx") == 0) {
        printf("%s: chdir dd/xx succeeded!\n", s);
        exit(1);
    }

    if (unlink("dd/dd/ffff") != 0) {
        printf("%s: unlink dd/dd/ff failed\n", s);
        exit(1);
    }
    if (unlink("dd/ff") != 0) {
        printf("%s: unlink dd/ff failed\n", s);
        exit(1);
    }
    if (unlink("dd") == 0) {
        printf("%s: unlink non-empty dd succeeded!\n", s);
        exit(1);
    }
    if (unlink("dd/dd") < 0) {
        printf("%s: unlink dd/dd failed\n", s);
        exit(1);
    }
    if (unlink("dd") < 0) {
        printf("%s: unlink dd failed\n", s);
        exit(1);
    }
}


// 尝试给内核传输非法字符串地址
void copyinstr(char *s) {
    uint64 addrs[] = {0x80000000LL, 0xffffffffffffffff};

    for (int ai = 0; ai < 2; ai++) {
        uint64 addr = addrs[ai];

        printf("打开文件，地址参数为%p\n", addr);
        int fd = open((char *)addr, O_CREATE | O_WRONLY);
        if (fd >= 0) {
            printf("open(%p) returned %d, not -1\n", addr, fd);
            exit(1);
        }
        else {
            printf("文件描述符 fd < 0, 拒绝访问\n");
        }
    }
}

void printpgtbl(char* s) {
    printf("打印当前进程的页表\n");
    vmprint();

    printf("申请一定量内存后，再打印一次\n");
    sleep(30);

    char* ptr = 0;
    int cnt = 0;
    while (cnt < 500) {
        cnt++;
        ptr = (char*)malloc(4096);
        if (cnt % 100 == 0) {
            printf("生成了 %d 个 page.\n", cnt);
        }
        *(ptr + 3) = '9';
    }

    vmprint();
    exit(0);
}


/*
    以下内容是辅助函数。
*/


// fork 失败，输出原因并退出程序。
void forkfailed(char* s) {
    printf("%s: fork 失败\n", s);
    exit(1);
}

// 运行测试的函数
int run(void f(char *), char* s) {
    pid_t pid;
    int xstatus;

    printf("\n-----------测试: %s-----------\n\n", s);
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
    struct test {
        void (*f)(char *);
        char* s;
        char* argname;
    } tests[] = {
        { MAXVAplus, "访问超出内存限制的地址", "maxva" },
        { memalloc, "内存无限增长", "memalloc" },
        { kernmem, "读内核地址", "kernmem" },
        { bsstest, "bss 段是否为 0", "bsstest" }, 
        { stacktest, "栈溢出保护", "stacktest" },
        { forktest, "fork 测试", "forktest" },

        { createfile, "文件创建", "createfile" },
        { simplefs, "文件读写", "simplefs" },
        // { subdir, "文件夹创建", "subdir" },
        { copyinstr, "读取非法字符串地址", "straddr" },
        { printpgtbl, "打印当前进程页表", "pgtbl" },

        { 0, 0, 0 },
    };

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            int f = 0;
            for (struct test *t = tests; t->argname != 0; t++) {
                if (strcmp(argv[i], t->argname) == 0) {
                    run(t->f, t->s);
                    f = 1;
                }
            }
            if (f == 0) {
                printf("未知程序: %s\n", argv[i]);
            }
        }
        exit(0);
    }

    printf("\n-----------开始内核功能测试-----------\n\n");

    for (struct test *t = tests; t->s != 0; t++) {
        run(t->f, t->s);
    }

    printf("-----------全部测试完成-----------\n");


    exit(0);
}