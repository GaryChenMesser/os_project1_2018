#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h> // O_CREATE
#include <sys/mman.h> // PROT_READ, PROT_WRITE, MAP_ANONYMOUS, MAP_SHARED
#include <unistd.h> // fork()
#include <sched.h> // sched_setscheduler()
#include <sys/time.h>
#include <linux/kernel.h>

#include "mysched.h"

// wait for one unit of time
#define wait_unit(unit) ({ for(size_t i = 0; i < unit; ++i)wait_one_unit; })
// be careful, it is i++ not ++i!
#define wait_one_unit ({ volatile unsigned long i; for(i = 0; i < 1000000UL; i++); })
// convenient way to convert string into policy number
#define policy(S) (POLICY[(int)S%7%4])
// If not in debug mode, uncomment this line.
#define DEBUG
#define PROTECTION (PROT_READ | PROT_WRITE)
#define VISIBILITY (MAP_ANONYMOUS | MAP_SHARED)

// unshared global variables:
// We have to create SJF and PSJF as kernel module because they're not pre-built in kernel.
// The number 1, 2, 5, 6 must be recheck for enumeration of schedular/
// 1->FIFO, 2->RR, which is pre-vuilt in kernel.
// 5->SJF, 6->PSJF, need to set by us.
int POLICY[4] = {1, 2, 5, 6};
typedef enum { CLOCK, REMAIN } GLOBAL_V;

// shared global variables:
unsigned long _c = 0, _r = 0;
unsigned long *_clock = &_c, *_remain =&_r;
sem_t *mutex, *wrt, *cond1, *cond2;

int main(){
	#ifdef DEBUG
	// disable buffering on stdout, which make printf in order.
	setbuf(stdout, NULL);
	#endif
	
	// parsing the input
	char S[4];
	int N;
	scanf("%s\n%d", S, &N);
	
	char P[N][32];
	int R[N], T[N];
	for(size_t i = 0; i < N; ++i){
		scanf("%s %d %d", P[i], &R[i], &T[i]);
	}

	// Initialize arrays to record sorted order.
	int R_index[N], T_index[N], R_inverse[N], T_inverse[N];
	for(size_t i = 0; i < N; ++i)
		R_index[i] = T_index[i] = i;
	
	// restrict to one cpu usage
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	
	cpu_set_t mask1;
	CPU_ZERO(&mask1);
	CPU_SET(1, &mask1);
		
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1)){
		printf("sched_setaffinity error: %s\n", strerror(errno));
		exit(1);
	}
	
	// set the scheduler
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(policy(S[0]));
	
	if(sched_setscheduler(0, policy(S[0]), &param)){
		printf("sched_setscheduler error: %s\n", strerror(errno));
		exit(1);
	}
	
  	// sort the ready and execution time, record the index in *_index
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);
	
	// ready now
	pid_t pid;
	unsigned long local_clock = 0;
	unsigned long i = 0;
	struct timespec start, end;
	
	for(0; i < N; ++i){
		while(local_clock != R[i]){
			wait_one_unit;
			++local_clock;
		}
		
		printf("%s forks!\n", P[R_index[i]]);
		getnstimeofday(&start);
		pid = fork();
		if(!pid){
			// restrict to one cpu usage
			if(sched_setaffinity(0, sizeof(cpu_set_t), &mask)){
				printf("sched_setaffinity error: %s\n", strerror(errno));
				exit(1);
			}
			goto CHILD;
		}
		else if(pid == -1){
			printf("Fork error!\n");
			exit(1);
		}
	}
	
	goto PARENT;
	
CHILD:
	wait_unit(T[T_inverse[R_index[i]]]);
	getnstimeofday(&end);
	printk("[Project1] %s %.9f %.9f", P[R_index[i]], start.tv_sec + start.tv_nsec / 10^9, end.tv_sec + end.tv_nsec / 10^9);
	printf("%s terminates!\n", P[R_index[i]]);
	exit(0);
	
PARENT:
	while(wait(NULL) > 0);
	
	
	sem_unlink("/mymutex");
	sem_unlink("/mywrt");
	sem_unlink("/mycond");
printf("main terminated.\n");
	exit(0);
}

