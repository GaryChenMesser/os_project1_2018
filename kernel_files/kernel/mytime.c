#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/time.h>

asmlinkage void sys_print(char p[], struct timespec * start, struct timespec * end){
    printk("[Project1] %s %.9f %.9f\n", p, start->tv_sec.tm_sec + start -> tv_nsec / 10^9, end->tv_sec.tm_sec + end->tv_nsec / 10^9);
}

asmlinkage void sys_gettime(struct timespec * ts){
    getnstimeofday(ts);
}
