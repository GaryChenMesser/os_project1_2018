#include <stdio.h>
#include <semaphore.h>
#include <assert.h>
#include "mysched.h"

// sorting function for recording indices
// 1 for ascending, 0 for descending
void swap(int * a, int * b){
	int c = *a;
	*a = *b;
	*b = c;
}

void sort(int value[], int index[], int N, int ascend){
	if(ascend){
		for(unsigned i = 0; i < N; ++i){
			for(unsigned j = i + 1; j < N; ++j){
				if(value[i] > value[j]){
					swap(&value[i], &value[j]);
					swap(&index[i], &index[j]);
				}
			}
		}
	}
	else{
		for(unsigned i = 0; i < N; ++i){
			for(unsigned j = i + 1; j < N; ++j){
				if(value[i] < value[j]){
					swap(&value[i], &value[j]);
					swap(&index[i], &index[j]);
				}
			}
		}
	}
}

void inverse_permutation(const int a[], int b[], int N){
	int count = 0, cursor = 0, probe = 0;
	for(unsigned i = 0; i < N; ++i)
		b[i] = -1;
	
	while(count < N){
		if(b[a[probe]] == -1){
			b[a[probe]] = probe;
			probe = a[probe];
			++count;
		}
		else{
			for(; cursor < N; ++cursor)
				if(b[cursor] == -1){
					probe = cursor;
					break;
				}
		}
	}
}

// flag == 1 for reading clock
// flag == 0 for reading remain
unsigned long reader(unsigned long * var, sem_t * mutex, sem_t * wrt){
	unsigned long temp;
	unsigned read_count = 0;
	
	// lock
	sem_wait(mutex);
	if(++read_count == 1)
		sem_wait(wrt);
	sem_post(mutex);
//printf("In critical.\n");
	// critical section
	temp = *var;
	
	// unclock
	sem_wait(mutex);
	if(!--read_count)
		sem_post(wrt);
	sem_post(mutex);
//printf("Out critical.\n");
	return temp;
}

unsigned long writer(unsigned long * var, long add, sem_t * wrt){
	unsigned long temp;
	assert( add != 0 );
	
	// lock
	sem_wait(wrt);
	
	// critical section
	*var += add;
	temp = *var;
	assert( *var >= 0 );

	// unclock
	sem_post(wrt);
	
	return temp;
}
