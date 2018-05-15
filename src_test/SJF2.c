#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>//gettimeofday
#include <unistd.h>//fork
void swap(int * a, int * b){
	int c = *a;
	*a = *b;
	*b = c;
}
void sort(int value[], int index[], int N, int ascend){
	if(ascend){
		for(size_t i = 0; i < N; ++i){
			for(size_t j = i + 1; j < N; ++j){
				if(value[i] > value[j]){
					swap(&value[i], &value[j]);
					swap(&index[i], &index[j]);
				}
			}
		}
	}
	else{
		for(size_t i = 0; i < N; ++i){
			for(size_t j = i + 1; j < N; ++j){
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
	for(size_t i = 0; i < N; ++i)
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
void SJF(int *R, int *R_index, int *T, int *T_inverse, int N) 
{
	
	
	int ready = 0;//# of process arrived and not finished
	int count = 0;//# of process arrived
	
	int startTime = 0;
	//struct timeval t1, t2;

	for (size_t i = 0; i < N; i++) 
	{
		while(count < N && R[count] <= startTime)
		{
			count++;
			ready++;
		}
		if (ready == 0)//continue 
		{
			startTime = R[count];
		}
		while(count < N && R[count] <= startTime) 
		{
			count++;
			ready++;
		}
		int shortest = -1;//find the shortest burst time process index
		for (size_t j = 0; j < count; j++) 
		{
			if (T[T_inverse[R_index[j]]] != 0)//process unfinished 
			{
				if (shortest == -1) 
				{
					shortest = R_index[j];
				}
				else if(T[T_inverse[R_index[j]]] < T[T_inverse[shortest]])
				{
					shortest = R_index[j];
				}
			}
		}
		/*
		pid_t pid = fork();
		if(pid == 0)
		{
			gettimeofday(&t1, NULL);
			for(size_t w = 0; w < T[T_inverse[shortest]]; ++w)
			{
				wait_one_unit;
			}
			gettimeofday(&t2, NULL);
			printf("[Project1] %d %d.%d %d.%d\n", getpid(), t1.tv_sec, t1.tv_usec, t2.tv_sec, t2.tv_usec);
			exit(0);
		}
		else if(pid < 0)
		{
			printf("Fails\n");
		}
		else
		{
			//parent
		}
		*/
		printf("\n%d", shortest);
		startTime += T[T_inverse[shortest]];
		T[T_inverse[shortest]] = 0;//finished
		ready--;
	}
}
int main()
{
	char S[4];
	int N;
	
	scanf("%s", S);
	scanf("%d", &N);
	
	char P[N][32];
	int R[N], T[N];
	int R_index[N], T_index[N], R_inverse[N], T_inverse[N];
	for(size_t i = 0; i < N; ++i)
		R_index[i] = T_index[i] = i;
	
	for(size_t i = 0; i < N; ++i){
		scanf("%s", P[i]);
		scanf("%d", &R[i]);
		scanf("%d", &T[i]);
	}
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);

	printf("\nR: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", R[i]);
	}
	printf("\nR_index: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", R_index[i]);
	}
	printf("\nR_inverse: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", R_inverse[i]);
	}
	printf("\nT: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", T[i]);
	}
	printf("\nT_index: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", T_index[i]);
	}
	printf("\nT_inverse: ");
	for(size_t i = 0; i < N; ++i){
		printf("%d ", T_inverse[i]);
	}


	SJF(R, R_index, T, T_inverse, N);


	return 0;
}




