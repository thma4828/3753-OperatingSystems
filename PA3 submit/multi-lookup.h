#ifndef MULTI_LOOKUP
#define MULTI_LOOKUP 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define ERROR1 99
#define port 8000

#define READ_MODE "r"
#define WRITE_MODE "w"
#define BOUND 15
#define MAX_CONS 3
#define MAX_PROD 3

#define MAX_ALLOCS 1000
#define MAXLINELENGTH 50

typedef struct{ //track all pointers I assign to the heap so that I dont memory leak. 
	char *char_addr_on_heap[MAX_ALLOCS];
	pthread_t *pt_addr_on_heap[MAX_ALLOCS];
	int cc;
	int cp;
}heap_address_tracker;

typedef struct{
	int value;
	pthread_t *id_array;
	int counter;
}semaphore;


void wait_s(semaphore *S){
	
	S->value = S->value - 1;
	if(S <= 0){
		
		pthread_mutex_unlock(&mutex_sh);
		pthread_cond_wait(&thread_condition, &mutex_sh);
	}
}

void signal_s(semaphore *S){
	
	S->value = S->value + 1;
	if(S->value > 0){
		pthread_cond_broadcast(&thread_condition);
	}
}



#endif //MULTI_LOOKUP
