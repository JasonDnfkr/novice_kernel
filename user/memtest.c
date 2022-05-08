#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"


int main() {
    char* ptr = 0;
    ptr = (char*)malloc(sizeof(10));
    ptr[10] = '\0';

    ptr[0] = 'a';

    printf("%s\n", ptr);

    int i = 0; 
    while (1) {
        // char* p = ptr;
        const int PGSIZE = 4096;
        ptr = (char*)malloc(sizeof(PGSIZE));
        ptr[6] = 'd';
        i++;
        if (i % 100000 == 0) {
            printf("%d", i);
        }
    }


    exit(0);
}