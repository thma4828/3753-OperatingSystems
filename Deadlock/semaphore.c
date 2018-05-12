#include <stdio.h>

struct semaphore{
	int val;
}

struct mutex{
	int available;
}

void wait(semaphore *S){
	while(S->val <= 0){ //busy wait
	}

	S->val--;
}

void signal(semaphore *S){
	S->val++;
}

void aquire(){
	while(!available){ //busy wait
	}
	available = 0;
}

void release(){
	available = 1;
}

/*A semaphore has internal state that is initialized to the number of available items of a given resource,
where mutex locks store only a boolean state of if the lock is taken or not. Therefore sempaphores are useful in situations
where more than 1 process needs access to a resource type with more than 1 instance. */