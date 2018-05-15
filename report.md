# 2018 OS Project 1 Report

Group 2

- 陳政曄
- 鄧聿晰
- 劉安齊

## Design

### Scheduler

// 排成邏輯設計

### System Call

There is only one system call in `kernel_file/sys_my_time.c`.

It is designed as the following:

```c
asmlinkage int sys_my_time(int isStart, unsigned long *start_sec,
                           unsigned long *start_nsec, unsigned long *end_sec,
                           unsigned long *end_nsec, int *pid)
```

The system call should called twice. The first is at just when the process is begining, and the second is at the moment the process has finished.

The usage as:

```c
struct  start_n, end_n; // Record time

// child process begins
syscall(350,                // System Call Number
        1,                  // Flag as "Start" <--- use
        &start_n.tv_sec,    // Start s         <--- use
        &start_n.tv_nsec,   // Start ns        <--- use
        &end_n.tv_sec,      // End s
        &end_n.tv_nsec,     // End ns
        &this_pid           // PID
        );

// ....
// A lot of work
// ....

syscall(350,                // System Call Number
        0,                  // Flag as "End"     <--- use
        &start_n.tv_sec,    // Start s
        &start_n.tv_nsec,   // Start ns
        &end_n.tv_sec,      // End s             <--- use
        &end_n.tv_nsec,     // End ns            <--- use
        &this_pid           // PID               <--- use
        );

// Child process exit
```

As you can see, there is `isStart` to determine whether it is beginning or ending. When it is beginning, it only records the start time. Otherwise, it records the end time, and call `printk` to print the information.

To record time, we can use both `getnstimeofday()` or `ktime_get()`. The prevois one is system time in ns, and the second one is time after system boots. Since we only need to know the order and duration, we can use either one to get the precision in nanosecond. I choose `ktime_get()` at the end, I explain in "Developing record" part.

### Simulator

// 模擬設計

## Running Result

## Discussion

## Contribution

// 各自填

### 陳政曄

### 鄧聿晰

### 劉安齊

Study the kernel files, and try to modify the scheduler in the kernel. Just after spending lots of time figure out the way to implement scheduler in kernel, we found
the homework is USER SPACE scheduler. So, well, ... Good try!

I also write and design the system call, and spending hours try to figure out why I cannot use `getnstimeofday()`. We will talk about the detail at the last section.

Teaching group how to use Git and Github.

## Developing record 開發辛酸史

### Kernel Scheduler

At the beginning, we thouth we need to modify scheduler in kernel, because we didn't see "USER SPACE". We know the another class need to implement "weighted rr" policy in kernel, and they have a handout. We took a look how they implement new schedule policy in linux kernel version `2.6.x`. However, the current linux kernel is `4.17.x`, so the code is very different from `2.6` to `4.17`. Also, knowing how to do "weighted rr" is helpless, becuase `SJF` anf `PSJF` do not need slice time. They are more likely `FIFO`. Anyway, I was on the way to add new policies in kernel `4.17`. There are some works, such as [this](https://github.com/GaryChenMesser/os_project1_2018/pull/6/files) and [this](https://github.com/GaryChenMesser/os_project1_2018/pull/8/files). However I got the information that we need not modify the kernel, and then I didn't do the following work. If we want to continue do a kernel scheduler, just base on the PR #8 and modify `rt.c`.

TL;DR 白忙一場了 Orz

### System call

To record the kernel time, at the beginning, we had tried to use `getnstimeofday()` for a long time, but it alway shows error while compiling. We have no idea and also open [a question on Stackover Flow](https://stackoverflow.com/questions/50349294/). We can't find solution. So, we change to use `ktime_get()`, which can compile. Just at the moment when I write the report now. I got the answer from Stackover Flow.

The origin system call is someting like this:

```c
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/time.h>

asmlinkage int sys_my_time() {
  struct timespec t;
  getnstimeofday(&t);
  // ...
  return 0;
}
```

and we finally know that the header should be:

```c
#include <linux/ktime.h>
#include <linux/timekeeping.h>
```

rather than `<linux/time.h>`.

Since the version 3.17, the function has moved. There is no issue on the Internet, so we did a good job, opening a quesion on Stackover Flow. :)
