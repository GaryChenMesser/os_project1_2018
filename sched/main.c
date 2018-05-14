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
#include <linux/list.h> 

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
// The number 1, 2, 7, 8 must be recheck for enumeration of schedular/
// 1->FIFO, 2->RR, which is pre-vuilt in kernel.
// 7->SJF, 8->PSJF, need to set by us.
int POLICY[4] = {1, 2, 7, 8};
typedef enum { CLOCK, REMAIN } GLOBAL_V;
struct ready_queue{
	struct list_head list;
	long start, exe;
	pid_t pid;
};

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
	cpu_set_t mask, mask1;

	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	
	CPU_ZERO(&mask1);
	CPU_SET(1, &mask1);

	// restrict the main process to execute on cpu 1
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1)){
		printf("sched_setaffinity error: %s\n", strerror(errno));
		exit(1);
	}
	
	// set the scheduler
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(policy(S[0]));
	// set once and all the child process will inherit the settings
	if(S == "FIFO" || S == "RR"){
		if(sched_setscheduler(0, policy(S[0]), &param)){
			printf("sched_setscheduler error: %s\n", strerror(errno));
			exit(1);
		}
	}
	
  	// sort the ready and execution time, record the index in *_index
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);
	
	if(S == "SJF" || S == "PSJF")
		resort(R, R_index, N, 1, T, T_inverse);
	
	// ready now
	pid_t pid;
	unsigned long local_clock = 0;
	unsigned long i = 0;
	struct timeval start, end;
	struct timezone zone;
	struct ready_queue ready, *tmp;
	struct list_head *pos, *q;
	
	INIT_LIST_HEAD(&ready.list);
	
	for(0; i < N; ++i){
		
		// wait until the first child to be forked
		while(local_clock != R[i]){
			wait_one_unit;
			++local_clock;
		}
		
		
		// check if head terminate
		tmp = list_entry(ready.list -> next, struct ready_queue, list);
		assert( tmp->start + tmp->exe <= R[i] );
		// if head terminates, add the shortest one to head
		if(tmp->start + tmp->exe == R[i])
		long shortest 
		list_for_each_safe(pos, q, &ready.list){
			tmp = list_entry(pos, struct ready_queue, list);
			if(tmp)/
		}
		
		
		int empty = list_empty_careful(&ready.list);
		
		tmp = (struct ready_queue*)malloc(sizeof(struct ready_queue));
		tmp->start = (empty) ? R[i] : -1;
		tmp->exe = T[T_inverse[R_index[i]]];
		list_add_tail(&(tmp->list), &(ready.list));
		
		
		printf("%s forks!\n", P[R_index[i]]);
		pid = fork();
		gettimeofday(&start, &zone);
		
		if(!pid){
			if(S == "SJF" || S == "PSJF"){
				int _policy = (empty) ? SCHED_FIFO : SCHED_IDLE;
				if(sched_setscheduler(0, _policy, &param)){
					printf("sched_setscheduler error: %s\n", strerror(errno));
					exit(1);
				}
			}
			// restrict all child processes to be executed on cpu 0
			printf("%s %d\n", P[R_index[i]], getpid());
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
		else{
			// parent
			tmp->pid = pid;
		}
	}
	
	goto PARENT;
	
CHILD:
	for(unsigned long _i = 0; _i < T[T_inverse[R_index[i]]]; ++_i){
		wait_one_unit;
	}
	//wait_unit(T[T_inverse[R_index[i]]]);
	
	gettimeofday(&end, &zone);
	printf("%s terminates!\n", P[R_index[i]]);
	syscall(334, P[R_index[i]], start, end);
	exit(0);
	
PARENT:
	while(wait(NULL) > 0);
	
	sem_unlink("/mymutex");
	sem_unlink("/mywrt");
	sem_unlink("/mycond");
	printf("main terminated.\n");
	exit(0);
}

