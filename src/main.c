#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> // fork()
#include <sched.h> // sched_setscheduler()
#include <sys/time.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include "list.h" 
#include "mysched.h"

// unshared global variables:
// 1->FIFO, 2->RR, which is pre-vuilt in kernel.
int POLICY[2] = {1, 2};

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
		#ifdef DEBUG
		printf("sched_setaffinity error: %s\n", strerror(errno));
		#endif 
		exit(1);
	}
	
	// set the scheduler
	struct sched_param param, param0;
	param.sched_priority = sched_get_priority_max((S[0] == 'R') ? SCHED_RR : SCHED_FIFO);
	param0.sched_priority = 0;
	
	// set once and all the child process will inherit the settings
	if(S[0] == 'F' || S[0] == 'R'){
		if(sched_setscheduler(0, policy(S[0]), &param)){
			#ifdef DEBUG
			printf("1 sched_setscheduler error: %s\n", strerror(errno));
			#endif 
			exit(1);
		}
	}
	
  	// sort the ready and execution time, record the index in *_index
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);
	if(S[0] == 'S' || S[0] == 'P')
		resort(R, R_index, N, 1, T, T_inverse);
	
	// ready now
	pid_t pid;
	unsigned long local_clock = 0;
	unsigned long i = 0;
	int this_pid;
	struct timeval start, end;
	struct timespec start_n, end_n;
	struct ready_queue ready, *tmp, *tmp1;
	
	INIT_LIST_HEAD(&ready.list);
	
	for(; i < N; ++i){
		int empty, preempt = 0;
		
		// wait until next child to be forked
		// check if there is any child possibly going to terminate
		// this implementation only rely on main process's local clock
		while(local_clock != R[i]){
			if((S[0] == 'S' || S[0] == 'P') && !list_empty(&ready.list))
				check_terminate(&ready, &param, local_clock);
			
			wait_one_unit;
			++local_clock;
		}
		
		// check if ready queue is empty		
		empty = list_empty(&ready.list);
		// add new instance to ready queue
		tmp = (struct ready_queue*)malloc(sizeof(struct ready_queue));
		tmp->start = (empty) ? R[i] : -1;
		tmp->exe = T[T_inverse[R_index[i]]];
		list_add_tail(&(tmp->list), &(ready.list));
		
		if(S[0] == 'P'){
			if(!list_empty(&ready.list))
				check_terminate(&ready, &param, local_clock);
			tmp1 = check_preempt(&ready, tmp, local_clock, preempt);
		}
		
		gettimeofday(&start, NULL);
		pid = fork();
		
		if(!pid){
			this_pid = getpid();
			syscall(350, 1, &start_n.tv_sec, &start_n.tv_nsec, &end_n.tv_sec, &end_n.tv_nsec, &this_pid);
			
			// restrict all child processes to be executed on cpu 0
			if(sched_setaffinity(0, sizeof(cpu_set_t), &mask)){
				#ifdef DEBUG
				printf("sched_setaffinity error: %s\n", strerror(errno));
				#endif 
				exit(1);
			}
			if(S[0] == 'S' || S[0] == 'P'){
				if(empty){
					if(sched_setscheduler(0, SCHED_FIFO, &param)){
						#ifdef DEBUG
						printf("policy: %d, sched_setscheduler error: %s\n", SCHED_FIFO, strerror(errno));
						#endif 
						exit(1);
					}
				}
				else{
					if(S[0] == 'P' && preempt){
						// need to add to fifo first, to avoid empty fifo ready queue,
						// which will make idle start to run!
						if(sched_setscheduler(0, SCHED_FIFO, &param)){
							#ifdef DEBUG
							printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
							#endif 
							exit(1);
						}
						if(sched_setscheduler(tmp1->pid, SCHED_IDLE, &param0)){
							#ifdef DEBUG
							printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
							#endif 
							exit(1);
						}
          }
					else{
						if(sched_setscheduler(0, SCHED_IDLE, &param0)){
							#ifdef DEBUG
							printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
							#endif 
							exit(1);
						}
					}
				}
			}
			for(unsigned long _i = 0; _i < T[T_inverse[R_index[i]]]; ++_i){
				wait_one_unit;
			}
	
			gettimeofday(&end, NULL);
			#ifdef DEBUG
			printf("[Project1] %d, %s %.6f %.6f\n", getpid(), P[R_index[i]], ((double)start.tv_sec + (double)start.tv_usec / (10^6)), ((double)end.tv_sec + (double)end.tv_usec / (10^6)));
			#endif 
			printf("%s %d\n", P[R_index[i]], this_pid);
			syscall(350, 0, &start_n.tv_sec, &start_n.tv_nsec, &end_n.tv_sec, &end_n.tv_nsec, &this_pid);
			exit(0);
		}
		else if(pid == -1){
			#ifdef DEBUG
			printf("Fork error!\n");
			#endif 
			exit(1);
		}
		else{
			tmp->pid = pid;
			
			if((S[0] == 'S' || S[0] == 'P') && !list_empty(&ready.list))
				check_terminate(&ready, &param, local_clock);
		}
	}
	
	// after all children have been forked, consume the remaining children which are still in idle.
	while(!list_empty(&ready.list) && (S[0] == 'S' || S[0] == 'P')){
		check_remain(&ready, &param, &local_clock);
	}
	// after all children have entered fifo, wait for the last on to terminate.
	while(wait(NULL) > 0);
	#ifdef DEBUG
	printf("main terminated.\n");
	#endif 
	exit(0);
}

