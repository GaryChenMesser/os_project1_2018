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

// wait for one unit of time
#define wait_unit(unit) ({ for(size_t i = 0; i < unit; ++i)wait_one_unit; })
// be careful, it is i++ not ++i!
#define wait_one_unit ({ volatile unsigned long i; for(i = 0; i < 1000000UL; i++); })
// convenient way to convert string into policy number
#define policy(S) (POLICY[(int)S%7%4])
// If not in debug mode, uncomment this line.
//#define DEBUG

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
		printf("sched_setaffinity error: %s\n", strerror(errno));
		exit(1);
	}
	
	// set the scheduler
	struct sched_param param, param0;
	param.sched_priority = sched_get_priority_max((S[0] == 'R') ? SCHED_RR : SCHED_FIFO);
	param0.sched_priority = 0;
	
	// set once and all the child process will inherit the settings
	if(S[0] == 'F' || S[0] == 'R'){
		if(sched_setscheduler(0, policy(S[0]), &param)){
			printf("1 sched_setscheduler error: %s\n", strerror(errno));
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
	struct timeval start, end;
	struct ready_queue ready, *tmp;
	
	INIT_LIST_HEAD(&ready.list);
	
	for(; i < N; ++i){
		int empty;
		
		// wait until next child to be forked
		// check if there is any child possibly going to terminate
		// this implementation only rely on main process's local clock
		while(local_clock != R[i]){
			if((S[0] == 'S' || S[0] == 'P') && !list_empty(&ready.list)){
				// get the running(or possibly terminated) child.
				tmp = list_entry(ready.list.next, struct ready_queue, list);
				
				// if this child terminates, remove it and add the shortest one to head
				assert( local_clock <= tmp->start + tmp->exe );
				if(tmp->start + tmp->exe == local_clock && (i == N - 1 || R[i] != R[i+1])){
					// remove
					list_del(&(tmp->list));

					// find the shortest and move it to the first of queue
					tmp = find_shortest(&ready);
					tmp->start = local_clock;

					if(sched_setscheduler(tmp->pid, SCHED_FIFO, &param)){
						printf("3 sched_setscheduler error: %s\n", strerror(errno));
						exit(1);
					}
				}
			}
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
		
		//printf("%s forks!\n", P[R_index[i]]);
		gettimeofday(&start, NULL);
		pid = fork();
		
		if(!pid){
			unsigned long start_sec, start_nsec, end_sec, end_nsec;
			int this_pid = getpid();
			// System define of 350, isStart
			syscall(350, 1, &start_sec, &start_nsec, &end_sec, &end_nsec, &this_pid);
			// restrict all child processes to be executed on cpu 0
			printf("%s %d\n", P[R_index[i]], getpid());
			if(sched_setaffinity(0, sizeof(cpu_set_t), &mask)){
				printf("sched_setaffinity error: %s\n", strerror(errno));
				exit(1);
			}
			if(S[0] == 'S' || S[0] == 'P'){
				if(empty){
					if(sched_setscheduler(0, SCHED_FIFO, &param)){
						printf("policy: %d, sched_setscheduler error: %s\n", SCHED_FIFO, strerror(errno));
						exit(1);
					}
				}
				else{
					if(S[0] == 'P'){
						struct ready_queue *tmp1 = list_entry(ready.list.next, struct ready_queue, list);
						assert( tmp1->start + tmp1->exe - local_clock > 0);
						tmp1->exe = tmp1->start + tmp1->exe - local_clock;
						if(tmp1->exe > tmp->exe){
							// need to add to fifo first, to avoid empty fifo ready queue,
							// which will make idle start to run!
							if(sched_setscheduler(0, SCHED_FIFO, &param)){
								printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
								exit(1);
							}
							if(sched_setscheduler(tmp1->pid, SCHED_IDLE, &param0)){
								printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
								exit(1);
							}
							list_del(&(tmp->list));
							list_add(&(tmp->list), &(ready.list));
						}
						
					}
					else{
						if(sched_setscheduler(0, SCHED_IDLE, &param0)){
							printf("policy: %d, sched_setscheduler error: %s\n", SCHED_IDLE, strerror(errno));
							exit(1);
						}
					}
				}
			}
			for(unsigned long _i = 0; _i < T[T_inverse[R_index[i]]]; ++_i){
				wait_one_unit;
			}
	
			gettimeofday(&end, NULL);
			printf("[Project1] %s %.6f %.6f\n", P[R_index[i]], ((double)start.tv_sec + (double)start.tv_usec / (10^6)), ((double)end.tv_sec + (double)end.tv_usec / (10^6)));
			// System call of 350, !isStart
			syscall(350, 0, &start_sec, &start_nsec, &end_sec, &end_nsec, &this_pid);
			exit(0);
		}
		else if(pid == -1){
			printf("Fork error!\n");
			exit(1);
		}
		else{
			tmp->pid = pid;
		}
	}
	
	// after all children have been forked, consume the remaining children which are still in idle.
	while(!list_empty(&ready.list) && (S[0] == 'S' || S[0] == 'P')){
		tmp = list_entry(ready.list.next, struct ready_queue, list);

		assert( local_clock <= tmp->start + tmp->exe );
		while(local_clock < tmp->start + tmp->exe){
			wait_one_unit;
			++local_clock;
		}
		list_del(&(tmp->list));
		if(!list_empty(&ready.list)){	
			tmp = find_shortest(&ready);
			tmp->start = local_clock;
			
			if(sched_setscheduler(tmp->pid, SCHED_FIFO, &param)){
				printf("3 sched_setscheduler error: %s\n", strerror(errno));
				exit(1);
			}
		}
	}
	// after all children have entered fifo, wait for the last on to terminate.
	while(wait(NULL) > 0);
	
	//printf("main terminated.\n");
	exit(0);
}

