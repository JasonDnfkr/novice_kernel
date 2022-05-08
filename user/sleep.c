#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        fprintf(2, "missing agrument. Usage: sleep [ticks]\n");
        exit(1);
    }
    else if (argc > 2) {
        fprintf(2, "expected argument. Usage: sleep [ticks]\n");
        exit(1);
    }
    else {
        int n = atoi(argv[1]);
        sleep(n);
    }

    exit(0);
} 