#include <stdio.h>

void producer(){
	while(1){
		wait(&empty); //block if buffer is full
		wait(&mutex);
		//buffer[IN] = new item
		signal(&mutex); //signal a blocked consumer
		signal(&full);
	}
}

void consumer(){
	while(1){
		wait(&full); //block if the buffer is empty
		wait(&mutex);
		//consume item from buffer
		signal(&mutex);
		signal(&empty); //signal a producer who is blocked on empty. 
	}
}

/* 
1. No deadlock
2. No busy wait
3. consumer blocks if buffer empty
4. producer blocks if buffer full 
*/