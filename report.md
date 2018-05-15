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

### Simulator

// 模擬設計

## Running Result

## Discussion

## Contribution

// 各自填

### 陳政曄

### 鄧聿晰

### 劉安齊
