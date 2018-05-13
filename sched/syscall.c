#include <stdio.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

int main(int argc, char** argv){
    struct timespec start, end;
    printf("%d\n", syscall(333, 1, 1));
    printf("%d\n", syscall(335, &start));
    printf("%d\n", syscall(335, &end));

    printf("%d\n", syscall(334, "p1", start, end));
    return 0;
}
