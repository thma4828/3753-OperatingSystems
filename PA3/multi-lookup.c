#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "multi-lookup.h"
#include "util.h"
#include "util.c"

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>

struct addr_info;
struct pool;
int dnslookup(const char*, char*, int); 

int main(int argc, char**argv){
	if(argc < 6){
		printf("usage: <#prod> <#cons> <req log> <res log> [<data files>]\n");
		exit(-1);
	}
	//start program timer
	struct timeval timer_init;
	struct timeval timer_final;
	gettimeofday(&timer_init, NULL);

	//initialize pool struct in global area of program = main stack. 
	struct pool ThreadPool;
	ThreadPool.num_prod = atoi(argv[1]);
	ThreadPool.num_cons = atoi(argv[2]);

	ThreadPool.file_count = 0;

    ThreadPool.output_files[0] = fopen(argv[4], "a"); //open  results.txt in append mode. 
    ThreadPool.output_files[1] = fopen(argv[3], "a"); //open serviced.txt ^^

    int M = 5;
    while(argv[M] != NULL){ //while there are still files to be passed in, pass those files into our files array, 
    	ThreadPool.files[M-5] = argv[M];
    	M++;
    	if(M-5 >= MAXFILES){ //no more than maxfiles files allowed.
    		break;
    	}
    }
    M = M - 5; //M = number of files. 

    ThreadPool.num_files = M;
    printf("number of files = %d\n", M);

	
	pthread_mutex_init(&ThreadPool.global_mutex, NULL);
	
	pthread_mutex_init(&ThreadPool.cons_write_lock, NULL);
	
	pthread_mutex_init(&ThreadPool.prod_write_lock, NULL);
	
	pthread_cond_init(&ThreadPool.consumer_block, NULL);
	
	pthread_cond_init(&ThreadPool.producer_block, NULL);

	ThreadPool.buffer_counter = 0; //intially buffer is empty
	ThreadPool.index = 0; //start at 0th index of buffer

	ThreadPool.is_writer_prod = 0; //initially no writer present 
	ThreadPool.is_writer_cons = 0; //^^^

	ThreadPool.producers_busy = 1; //intially both producers and consumers are working
	ThreadPool.consumers_busy = 1; //^^^

    for(int j=0; j<M; j++){
    	ThreadPool.open_files[j] = fopen(ThreadPool.files[j], "r");
    	if(ThreadPool.open_files[j] == NULL){
    		printf("at least 1 file failed to open\n --> exiting with code 13\n");
    		exit(13);
    	}
    	ThreadPool.files_done[j] = 0; //initially no files are done. 
    }

    for(int i=0; i<ThreadPool.num_prod; i++){ //CREATING PRODUCER THREADS
    	ThreadPool.prod_ids[i] = i;
    	pthread_create(&ThreadPool.prod_ids[i], NULL, producer, &ThreadPool);
    }

    for(int k=0; k<ThreadPool.num_cons; k++){ //CREATING CONSUMER THREADS
    	ThreadPool.cons_ids[k] = k;
    	pthread_create(&ThreadPool.cons_ids[k], NULL, consumer, &ThreadPool);
    }

    for(int j=0; j<ThreadPool.num_prod; j++){ //JOINING PRODUCER THREADS
    	pthread_join(ThreadPool.prod_ids[j], NULL);
    }

    for(int j=0; j<ThreadPool.num_cons; j++){ //JOINING CONSUMER THREADS
    	pthread_join(ThreadPool.cons_ids[j], NULL);
    }
    gettimeofday(&timer_final, NULL); //end program timer
    double time_difference = timer_final.tv_sec - timer_init.tv_sec; //calculate the difference
    printf("====== Elapsed time for total execution: %lf ======\n", time_difference); //print to STDOUT
    return 0;
}

void *producer(void *param){
	unsigned int tid = (unsigned int)pthread_self();
	printf("[PRODUCER [%u]]: init\n", tid);
	struct pool *thread_pool = (struct pool *)param; //get pointer to thread pool struct in main. 
	int fc = 0; //files served = 0;
	while(1){ //outer loop
		//crit section 1
		int lines_read = 0;
		pthread_mutex_lock(&thread_pool->global_mutex); //lock global mutex
		int K = 0;
		printf("[PRODUCER [%u]]: selecting a file\n", tid);
		printf("%d == num_files: ",thread_pool->num_files);
		while(K < thread_pool->num_files){
			if(thread_pool->files_done[K] == 1){
				K++;
				printf("k === %d: \n", K);
			}else{
				break;
			}
		}
		printf("k: %d", K);
		if(K >= thread_pool->num_files){
			thread_pool->producers_busy = 0;
			pthread_mutex_unlock(&thread_pool->global_mutex); //give up global mutex. 
			break; //producers are done! break out of the loop
		}
		FILE *pfile = thread_pool->open_files[K]; //get pointer to particular file K that is not done
		thread_pool->file_count = (thread_pool->file_count + 1) % MAXFILES;
		int which_file = thread_pool->file_count;
		pthread_mutex_unlock(&thread_pool->global_mutex); //unlock global mutex
		//end of crit section 1

		if(pfile == NULL){
			printf("file not open\n");
		}else{
			char *p;
			char f_line[LINE_LEN];
			
			while((p = fgets(f_line, LINE_LEN, pfile)) != NULL){
				//crit section 2: accessing the heap with malloc, then adding to the buffer
				
				int i=0; 
				while((i < LINE_LEN) && (p[i] != '\n')){ //determine actual length of string
					i++;
				}
				pthread_mutex_lock(&thread_pool->global_mutex); //lock global mutex
				char *hostname = malloc(i); //malloc a spot in the heap
				for(int z=0; z<i; z++){
					hostname[z] = p[z]; //allocate the name to the heap. 
				}
				printf("[PRODUCER [%u]]:	 hostname =%s.\n",tid, hostname);
				if(thread_pool->buffer_counter >= BUFFERSIZE){ //if buffer is full
					printf("[PRODUCER [%u]]:	buffer full\n",tid);
					pthread_mutex_unlock(&thread_pool->global_mutex); //give up mutex
					pthread_cond_signal(&thread_pool->consumer_block); //wakeup 1 consumer that is blocked on consumer block condition.
					printf("[PRODUCER [%u]]:	signal success\n",tid); 
					pthread_cond_wait(&thread_pool->producer_block, &thread_pool->global_mutex); // block producer thread
				} //note that after getting through this statement a producer will have the global mutex! 
				//bounded buffer problem crit section 3: already protected by global mutex
				printf("[PRODUCER [%u]]:	entering critical section\n", tid);
				thread_pool->buffer[thread_pool->index] = hostname;
				thread_pool->index = (thread_pool->index + 1) % BUFFERSIZE;
				thread_pool->buffer_counter = thread_pool->buffer_counter + 1;
				pthread_mutex_unlock(&thread_pool->global_mutex); //give up mutex claim on resources. 
				pthread_cond_signal(&thread_pool->consumer_block); //wakeup one consumer who is blocked on the buffer. 
				//end of producers bounded buffer sequence. 
				lines_read++;
			}
			
		}
		thread_pool->files_done[which_file] = 1;
		if(lines_read)
			fc++; //increment files serviced counter

	}
	//crit section 4
	pthread_mutex_lock(&thread_pool->prod_write_lock); //we gain access to producer writer mutex. 
	FILE *serviced_txt = thread_pool->output_files[1]; //serviced.txt pointer
	char *msg = "Thread serviced #files = \n"; //my message
	size_t bw1 = fwrite(msg, 1, strlen(msg), serviced_txt); //write to the file. 
	size_t bw2 = fwrite(&fc, 4, 1, serviced_txt); //write to the file number of files serviced. 
	pthread_mutex_unlock(&thread_pool->prod_write_lock); //end of critical section, release writer lock. */
	pthread_exit(0); //exit
}

void *consumer(void *param){
	unsigned int tid = (unsigned int)pthread_self();
	printf("[CONSUMER [%u]]:	init\n", tid);
	struct pool *thread_pool = (struct pool *)param; //ptr to thread pool
	while(1){ //conusmer main loop
		//critical section for consumers
		pthread_mutex_lock(&thread_pool->global_mutex);
		printf("[CONSUMER [%u]]:	entering critical section\n", tid);
		if(thread_pool->buffer_counter <= 0){ //buffer empty
			printf("[CONSUMER [%u]]:	buffer empty\n", tid);
			if(!thread_pool->producers_busy){ //if producers done with the files. 
				pthread_mutex_unlock(&thread_pool->global_mutex);  //give up mutex and break out of loop, program finished. 
				printf("[CONSUMER [%u]]:	buffer empty AND producers done --> TERMINATING CONSUMER\n", tid);
				break;
			}
			pthread_mutex_unlock(&thread_pool->global_mutex); //give up access to shared resources
			pthread_cond_signal(&thread_pool->producer_block); //wakeup 1 producer that is blocked on the buffer. 
			pthread_cond_wait(&thread_pool->consumer_block, &thread_pool->global_mutex); /*block on consumer condition and 
																						when woken up will have access to global mutex*/
		}
		//we can now have access to the buffer! 
		printf("[CONSUMER [%u]]:	getting item from buffer\n", tid);
		char *hostn = thread_pool->buffer[(thread_pool->index - 1) % BUFFERSIZE]; //get address from the buffer
		printf("[CONSUMER [%u]]:	decrementing index\n", tid);
		thread_pool->index = (thread_pool->index - 1) % BUFFERSIZE;
		thread_pool->buffer_counter = thread_pool->buffer_counter - 1;
		pthread_mutex_unlock(&thread_pool->global_mutex); //give up access to shared resources. (no hold and wait)
		pthread_cond_signal(&thread_pool->producer_block); //wakeup a producer who is blocked on producer condition

		//resolve to IP address and write to results.txt. 
		//crit section 2
		
		printf("[CONSUMER [%u]]: writing to file\n", tid);
		pthread_mutex_lock(&thread_pool->cons_write_lock); // we want to write the resolved IP address to a file. 
		FILE *results_txt = thread_pool->output_files[0]; //results.txt pointer
		if(results_txt)
			printf("[CONSUMER [%u]]: results_txt is non-null\n", tid);
		if(hostn != NULL){
			struct hostent *host_struct = gethostbyname(hostn);
				if(host_struct == NULL){
					printf("failed to resolve host name.\n");
				}else{
					char **addr_list = host_struct->h_addr_list;
					if(addr_list == NULL){
						printf("failed to resolve host name: %s \n", hostn);
					}else{
						if(addr_list[0]){
							char *current_addr = addr_list[0]; //4 byte IP address. 
							pthread_mutex_lock(&thread_pool->cons_write_lock); //get consumer write lock
							fprintf(results_txt, "hostname: %s address:  %02x\n", hostn, *current_addr);
							pthread_mutex_unlock(&thread_pool->cons_write_lock); //release the write lock. 
						}else{
							printf("failed to resolve host name: %s \n", hostn);
						}
					}
				}
				
		}else{
			printf("[CONSUMER [%u]]: item taken from buffer was NULL\n", tid);
		}
		pthread_mutex_unlock(&thread_pool->cons_write_lock); //give up writer mutex. 

	}
	pthread_exit(0); //exit
}