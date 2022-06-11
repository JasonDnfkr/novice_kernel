#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "user/user.h"
#include "kernel/include/fs.h"

#define MAXSTRLEN 10

const char* FNAME     = "Name           ";
const char* FTYPE     = "Type   ";
const char* FINODE    = "Ino ";
const char* FSIZE     = "Size (Bytes)";

const char* DIR       = "DIR    ";
const char* FILE      = "FILE   ";
const char* DEVICE    = "DEVICE ";
const char* SYMLINK   = "SYMLINK";


char* fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void lsfmtprint(char* buf, struct stat* st) {
    switch (st->type) {
    case T_FILE:
        if (st->ino < 10) {
            printf("%s  %s %d    %l\n", fmtname(buf), FILE, st->ino, st->size);
        }
        else {
            printf("%s  %s %d   %l\n", fmtname(buf), FILE, st->ino, st->size);
        }
        break;
        
    case T_DIR:
        if (st->ino < 10) {
            printf("%s  %s %d    %l\n", fmtname(buf), DIR, st->ino, st->size);
        }
        else {
            printf("%s  %s %d   %l\n", fmtname(buf), DIR, st->ino, st->size);
        }
        break;
    // default:
    //     break;
    // case T_DEVICE:
    // if (st->ino == 1) break;
    // if (st->ino < 10) {
    //         printf("%s  %s %d    %l\n", fmtname(buf), DEVICE, st->ino, st->size);
    //     }
    //     else {
    //         printf("%s  %s %d   %l\n", fmtname(buf), DEVICE, st->ino, st->size);
    //     }
    }
}

void ls(char *path) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    printf("--------------- ------- ---- ------------\n");
    printf("%s %s %s %s\n", FNAME, FTYPE, FINODE, FSIZE);
    printf("--------------- ------- ---- ------------\n");

    switch (st.type) {
    case T_FILE:
        // printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
        lsfmtprint(path, &st);
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0) {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            // printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            lsfmtprint(buf, &st);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        ls(".");
        exit(0);
    }
    for (i = 1; i < argc; i++)
        ls(argv[i]);
    exit(0);
}
