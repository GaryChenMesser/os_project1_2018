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

int main(){
	pid_t pid;
	
	pid = fork();
	
	if(!pid){
		printf("this is child. pid = %d\n", pid);
		pid = fork();
		if(!pid){
			printf("this is grandchild. pid = %d\n", pid);
		}
		else if(pid > 0){
			printf("this is child as parent. pid = %d\n", pid);
		}
		else{
		
		}
	}
	else if(pid > 0){
		printf("this is parent. pid = %d\n", pid);
		
	}
	else{
	
	}	
	
	exit(0);
}
