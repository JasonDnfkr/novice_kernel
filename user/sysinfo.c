#include "kernel/include/types.h"
#include "kernel/include/riscv.h"
#include "kernel/include/sysinfo.h"
#include "user/user.h"

void sinfo(struct sysinfo *info) {
    if (sysinfo(info) < 0) {
        printf("FAIL: sysinfo failed");
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    struct sysinfo si;
    sinfo(&si);
    printf("Free memory: %d\n", si.freemem);
    printf("Process count: %d\n", si.nproc);
    
    exit(0);
}
