# 2018 OS Project 1 Report

Group 2

- 陳政曄 @GaryChenMesser
- 鄧聿晰 @tigercosmos
- 劉安齊 @b05701204

## Design

本排程器分別使用不同的 policy 組合實現不同的排程：

- 使用 `SCHED_FIFO` 實現 `FIFO`
- 使用 `SCHED_FIFO` 搭配 `SCHED_IDLE` 完成 `RR`, `SJF` 和 `PSJF` 的實作。

排程器(主程式、父行程)本身會單獨使用一顆 cpu 並使用 `SCHED_OTHER` policy，以完成比較準確的時間單位計算並不被自己 fork 出來的子行程干擾。相對的，子行程共用另一顆 CPU，目的是讓 `SCHED_FIFO` `和SCHED_RR` 可以實現真正的行為，不被類似 pipeline 的效果影響。

由於主程式和子行程的 CPU 和 policy 都不一樣，因此兩者不會出現在同一個 ready queue 上，且不會互卡CPU時間(不同CPU)，因此可以達到接近平行化的效果，好處是可以直接在主程式和子行程跑計算時間的空迴圈，不會因為兩者同時跑導致時間伸縮。

### Scheduler

本排程器大致流程如下：
#### FIFO

- 主程式(父行程)
  - 把需要被執行與排程的行程進行排序(開始時間早的排前面)
  - 進入準備狀態
  - 開始計算時間，每當有行程需要被fork，就fork該行程
  - 直到所有子行程真正結束，本程式結束

#### SJF(PSJF僅一處不同)

##### 主程式(父行程)

- 把需要被執行與排程的行程進行排序(開始時間早的排前面，當開始時間一樣，會把結束時間早的排前面)。
- 進入準備狀態
- 開始計算時間
  - 每當有行程需要被 fork，就 fork 並把該子行程的資料丟入一個 queue 當中，並繼續算時間。
  - 每當有行程到了”表定”的結束時間，就把 queue 的第一個元素拿掉，把剩餘元素中執行時間最短的放到queue 的開頭(插隊)。
- 直到所有子行程真正結束，本程式結束

##### 子行程(子行程真正開始執行自己的任務前要做的routine)

- 把 CPU 鎖到第二顆 CPU 上
- 檢查繼承到的 queue 是否為空(檢查的部分為了避免多行程導致的臨界區問題，已在父行程檢查完畢，以下檢查只檢查存起來的 flag)
  - 為空
    - 把自己的 sched policy 改成 `SCHED_FIFO`
  - 不為空
    - 若是 `PSJF`，且剩餘時間大於 queue 開頭的行程的剩餘時間，把自己的 sched policy 改成`SCHED_FIFO`
    - 若否，把自己的 sched policy 改成 `SCHED_IDLE`

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

We have implemented the four scheduling algorithms followed with their procedure introduced in class. And we use the thereotical results to verify the real order created from running process.

## Running Result

## Discussion

## Contribution

// 各自填

### 陳政曄

### 鄧聿晰

Write scheduler simulator and create testdata outputs for checkout.

### 劉安齊

Study the kernel files, and try to modify the scheduler in the kernel. Just after spending lots of time figure out the way to implement scheduler in kernel, we found
the homework is USER SPACE scheduler. So, well, ... Good try!

I also write and design the system call, and spending hours try to figure out why I cannot use `getnstimeofday()`. We will talk about the detail at the last section.

Teaching group how to use Git and Github.

## Developing record 開發辛酸史

### Kernel Space Scheduler

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

then Booooom! Error????

```no
kernel/sys_my_time.c: In function ‘sys_my_time’:
kernel/sys_my_time.c:8:3: error: implicit declaration of function ‘getnstimeofday’ [-Werror=implicit-function-declaration]
getnstimeofday(&t);
^
```

We finally know that the header should be:

```c
#include <linux/ktime.h>
#include <linux/timekeeping.h>
```

rather than `<linux/time.h>`.

Since the kernel version `3.17`, the function has moved from `time.h` to `timekeeping.h`. Also, we cannot only import `<linux/timekeeping.h>`. The `<linux/ktime.h>` need to be bound together.

There is no discussion on the Internet, so we did a good job, opening a quesion on Stackover Flow and get the answer. :)
