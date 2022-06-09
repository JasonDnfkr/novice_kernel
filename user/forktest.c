// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"

// #define N 1000

// void print(const char *s) {
//     write(1, s, strlen(s));
// }

// void forktest(void) {
//     int n, pid;

//     print("fork test\n");

//     for (n = 0; n < N; n++) {
//         pid = fork();
//         if (pid < 0)
//             break;
//         if (pid == 0)
//             exit(0);
//     }

//     if (n == N) {
//         print("fork claimed to work N times!\n");
//         exit(1);
//     }

//     for (; n > 0; n--) {
//         if (wait(0) < 0) {
//             print("wait stopped early\n");
//             exit(1);
//         }
//     }

//     if (wait(0) != -1) {
//         print("wait got too many\n");
//         exit(1);
//     }

//     print("fork test OK\n");
// }

// int main(void) {
//     forktest();
//     exit(0);
// }

void fun(char* s) {
    enum { N = 1000 };
    int n, pid;
    printf("进程调度测试: \n");
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

int main(int argc, char* argv[]) {
    fun("forktest");
    exit(0);
}