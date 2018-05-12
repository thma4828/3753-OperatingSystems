#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
/* SOLUTION TO READERS WRITERS PROBLEM 2
NO WRITER KEPT WAITING INDEFINATELY ON READERS
NO READER KEPT WAITING FOR OTHER READERS*/
int wrt = 1;
int read_block = 1;
int mutex = 1;
int read_count = 0;


void writer(){
	while(1){
		wait(&read_block); //block new readers in
		wait(&wrt); //wait for exclusive access to write

		//write to file

		signal(&wrt); //give up write lock
		signal(&read_block); //let new readers in. 
	}
}

void reader(){
	while(1){
		wait(&read_block); //if writer has arrived all new readers will block
		wait(&mutex); //access to read_count variable
		read_count++;
		if(read_count == 1){ //first reader in
			wait(&wrt); //wait on the writer to give up the write lock / aquire the lock for other readers 
		}
		signal(&mutex);
		signal(&read_block);
		
		//read from file

		wait(&mutex);
		read_count--;
		if(read_count == 0){ //last reader out
			signal(&wrt);
		}
		signal(&mutex);
	}

}