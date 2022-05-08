#include "kernel/include/param.h"
#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int i;
    char* nargv[MAXARG];

    if (argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')) {
        fprintf(2, "Usage: %s mask command\n", argv[0]);
        exit(1);
    }

    if (strace(atoi(argv[1])) < 0) {
        fprintf(2, "%s: strace failed\n", argv[0]);
        exit(1);
    }

    for (i = 2; i < argc && i < MAXARG; i++) {
        nargv[i - 2] = argv[i];
    }

    // nargv[0] is syscall
    // nargv[1+] are next arguments

    exec(nargv[0], nargv);
    exit(0);
}
