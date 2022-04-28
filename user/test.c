#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    char str[10] = "test str\n";

    // uint64 addr = (uint64)str;

    // printf("<%p>\n", addr);

    write(1, str, strlen(str));    
    
    exit(0);
}