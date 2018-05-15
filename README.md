# OS Project 1 2018

1. Copy `kernel_file/sys_my_time.c` to `linux/kernel/sys_my_time.c`
2. Add line "`obj-y += my_time.o`" in `linux/kernel/Makefile`
3. Add line in "`linux/include/linux/syscalls.h`":
    ```c
    asmlinkage int sys_my_time(int isStart, unsigned long *start_sec,
                                unsigned long *start_nsec, unsigned long *end_sec,
                                unsigned long *end_nsec, int *pid);
    ```
4. Add `350 common my_time sys_my_time` in `linux/arch/x86/entry/syscalls/syscall_64.tbl`
5. `make -j4 bzImage`
6. `make install`
7. reboot
