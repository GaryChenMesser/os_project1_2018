#ifndef MYSCHED_H_
#define MYSCHED_H_

#include "list.h"

struct ready_queue{
	struct list_head list;
	long start, exe;
	pid_t pid;
};

// sorting function for recording indices
// 1 for ascending, 0 for descending
void swap(int * a, int * b);
void sort(int value[], int index[], int N, int ascend);
void resort(int value[], int index[], int N, int ascend, const int T[], const int T_inverse[]);

void inverse_permutation(const int a[], int b[], int N);

struct ready_queue * find_shortest(struct ready_queue *ready);
#endif
