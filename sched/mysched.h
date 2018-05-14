#ifndef MYSCHED_H_
#define MYSCHED_H_

// sorting function for recording indices
// 1 for ascending, 0 for descending
void swap(int * a, int * b);
void sort(int value[], int index[], int N, int ascend);
void resort(int value[], int index[], int N, int ascend, const int T[], const int T_inverse[]){

void inverse_permutation(const int a[], int b[], int N);
// flag == 1 for reading clock
// flag == 0 for reading remain
unsigned long reader(unsigned long * var, sem_t * mutex, sem_t * wrt);
unsigned long writer(unsigned long * var, long add, sem_t * wrt);

#endif
