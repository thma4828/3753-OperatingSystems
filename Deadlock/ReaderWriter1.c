#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
/* SOLUTION TO READERS WRITERS PROBLEM 1
NO READER KEPT WAITING FOR OTHER READERS
WRITERS COULD STARVE!*/
int wrt = 1;
int mutex = 1;
int read_count = 0;


void writer(){
	while(1){
		wait(&wrt); //wait for exclusive access to write

		//write to file

		signal(&wrt); //give up write lock
	}
}

void reader(){
	while(1){
		wait(&mutex); //access to read_count variable

		read_count++;
		if(read_count == 1){ //first reader in
			wait(&wrt); //aquire the lock since first reader in
		}

		signal(&mutex);

		//read from file

		wait(&mutex);

		read_count--;
		if(read_count == 0){ //last reader out
			signal(&wrt);
		}
		
		signal(&mutex);
	}

}
/*Note that the reason the writers could starve is because if new readers keep coming in
then the readers will never realease the write lock because read_count > 0*/