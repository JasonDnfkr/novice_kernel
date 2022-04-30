#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysinfo.h"

int main(int argc, char* argv[]) {
    char str[10] = "test str\n";

    write(1, str, strlen(str));    

    struct sysinfo si;
    sysinfo(&si);
    printf("mem: %d, proc: %d\n", si.freemem, si.nproc);
    
    exit(0);
}