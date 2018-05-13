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
	if(sched_setaffinity(0, sizeof(mask), &mask)){
		printf("Error: %s\n", strerror(errno));
		exit(1);
	}
	
	// set the scheduler
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(policy(S[0]));
	
	// Mapping global variables to shared address, which make them visible to unrelated processes
	// "clock" contains the central time for all the processes.
	unsigned long * clock = mmap(NULL, sizeof(unsigned long), PROTECTION, VISIBILITY, 0, 0);
	*clock = 0;
	// "remain" contains the number of total child processes.
	unsigned long * remain = mmap(NULL, sizeof(unsigned long), PROTECTION, VISIBILITY, 0, 0);
	*remain = 0;
	unsigned long * mainrest = mmap(NULL, sizeof(unsigned long), PROTECTION, VISIBILITY, 0, 0);
	*mainrest = 0;
	
	// Init mutexes and conditional variables.
	sem_t * mutex = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(mutex, 1, 1);
	sem_t * mutex2 = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(mutex2, 1, 1);
	sem_t * mutex3 = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(mutex3, 1, 1);
	sem_t * wrt = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(wrt, 1, 1);
	sem_t * cond1 = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(cond1, 1, 0);
	sem_t * cond2 = mmap(NULL, sizeof(sem_t), PROTECTION, VISIBILITY, 0, 0);
	sem_init(cond2, 1, 0);
	
  	// sort the ready and execution time, record the index in *_index
	sort(R, R_index, N, 1);
	sort(T, T_index, N, 1);
	inverse_permutation(R_index, R_inverse, N);
	inverse_permutation(T_index, T_inverse, N);
	
	// ready now
	pid_t pid;
	unsigned long ready_index = 0;

	for(size_t i = 0; i < N; ++i){
		int nonready = 1;
		do{
			if(!reader(remain, mutex, wrt)){
				wait_one_unit;
				if( !(nonready = (R[i] != writer(clock, 1, wrt)) ) ){
					
					sem_wait(mutex3);
					printf("%s forked. if pid = %d\n", P[R_index[i]], pid);
					writer(remain, 1, wrt);
					pid = fork();
					if(!pid){
						if(sched_setscheduler(0, policy(S[0]), &param)){
							printf("Error: %s\n", strerror(errno));
							exit(1);
						}
						sem_post(mutex3);
					}
				}
			}
			else{
				sem_wait(cond1);
				int v;
				sem_getvalue(cond1, &v);
				if( !(nonready = (R[i] != reader(clock, mutex, wrt)) ) ){
					do{
						sem_wait(mutex3);
						printf("%s forked. else pid = %d\n", P[R_index[i]], pid);
						writer(remain, 1, wrt);
						pid = fork();
						if(!pid){
							if(sched_setscheduler(0, policy(S[0]), &param)){
								printf("Error: %s\n", strerror(errno));
								exit(1);
							}
							sem_post(mutex3);
						}
						else{
							if(R[i + 1] == R[i]){
								++i;
							}
							else{
								break;
							}
						}
					}while(pid);
				}
				if(i == N - 1){
					writer(mainrest, 1, wrt);
				}
				if(pid){
				}
			}
		}while(pid && nonready);
		
		if (pid == 0){
			// This is the child process.
			// Wait for execution time.
			for(size_t w = 0; w < T[T_inverse[R_index[i]]]; ++w){
				wait_one_unit;
				writer(clock, 1, wrt);
				if(!reader(mainrest, mutex, wrt)){	
					sem_wait(mutex2);
					sem_wait(cond2);
					sem_post(mutex2);
				}
				else{
				}
			}
			// printk("%s ");
			printf("%s terminated.", P[R_index[i]]);
			writer(remain, -1, wrt);
			exit(0);
		}
		else if (pid < 0){
			// The fork failed.
			printf("Fails.\n");
		}
		else{
			// This is the parent process.
			if(i == N - 1){
				int wpid, v;
				while((wpid = wait(NULL)) > 0){
					if(!sem_getvalue(cond2, &v))
						sem_post(cond2);	
				}
			}
			continue;
		}
	}
	
	sem_unlink("/mymutex");
	sem_unlink("/mywrt");
	sem_unlink("/mycond");
printf("main terminated.\n");
	exit(0);
}

