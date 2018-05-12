#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

#define ERROR1 99


#define READ_MODE "r"
#define WRITE_MODE "w"

#define BOUND 15
#define MAX_CONS 10
#define MAX_PROD 10

#define MAX_ALLOCS 1000
#define MAXLINELENGTH 50

void *producer();
void *consumer();




typedef struct Thread_Pool{

	pthread_t prod_tid[10];
	pthread_t con_tid[10];

	//mutex for produces
	pthread_mutex_t mutex_sh = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t consumer_write = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutex_consumers = PTHREAD_MUTEX_INITIALIZER;

	pthread_cond_t thread_condition;

	int producers_done;

	char **names_read;
	
	char *shared_buffer[BOUND];
	unsigned int in;
	unsigned int out;
	unsigned int shb_count;

	FILE *open_files[5];

};

void wait_s(semaphore *S, Thread_Pool *T){
	//pthread_mutex_lock(&sema);
	S->value = S->value - 1;
	
	
	
	if(S <= 0){
		//pthread_mutex_unlock(&sema);
		pthread_mutex_unlock(&mutex_sh);
		pthread_cond_wait(&thread_condition, &mutex_sh);
	}
	//pthread_mutex_unlock(&sema);
	
}

void signal_s(semaphore *S, Thread_Pool *T){
	//pthread_mutex_lock(&sema);
	S->value = S->value + 1;
	

	
	if(S->value > 0){
		pthread_cond_signal(&thread_condition); //wakeup one thread blocked on thread condition
	}
	//pthread_mutex_unlock(&sema);
}


int main(int argc, char **argv){

   
    //int fd = open("/PA3/input/names1.txt", O_RDONLY);
    char *a1 = "/home/user/3753/PA3/input/names1.txt";
    char *a2 = "/home/user/3753/PA3/input/names2.txt";
    char *a3 = "/home/user/3753/PA3/input/names3.txt";
    char *a4 = "/home/user/3753/PA3/input/names4.txt";
    char *a5 = "/home/user/3753/PA3/input/names5.txt";

    char **files = malloc(8 * 5); //remember to free files at the end
    files[0] = a1;
    files[1] = a2;
    files[2] = a3;
    files[3] = a4;
    files[4] = a5;

    int num_files = 5;

    Thread_Pool POOL; //the thread pool
    POOL.names_read = malloc(8 * 400); //names in table. 

    for(int p=0; p<num_files; p++){
    	POOL.open_files[p] = fopen(files[p], READ_MODE);
    	if(POOL.open_files[p] == NULL)
    		printf("at least 1 file open failed\n");
    }

    for(int i=0; i<num_files; i++){ //5 producers
    	POOL.prod_tid[i] = i;
    	pthread_create(&(POOL.prod_tid[i]), NULL, producer, POOL.open_files[i], &POOL); //pass each thread an id, function to run on file to open and a pointer to the thread pool struct 
    }
    printf("producer threads created\n");
    

    //creating a thread pool for consumers
    
    
    for(int i=0; i<MAX_CONS; i++){ //10 consumers
    	con_tid[i] = i;
    	pthread_create(&(POOL.con_tid[i]), NULL, consumer, NULL);
    }
    printf("consumer threads created\n");


    //waiting for threads to complete
    for(int j=0; j<3; j++){
    	pthread_join(con_tid[j], NULL);
    }
    /*for(int l=0; l<BOUND; l++){
    	printf("shared buffer contents at shared_buffer[%d] = ptr to heap = %p\n", l, (void *)shared_buffer[l]);
    	printf("--->points to string: %s\n", shared_buffer[l]);
    }*/
    printf("consumer threads joined\n");
    for(int j=0; j<3; j++){
    	pthread_cond_broadcast(&producer_condition);
    	pthread_join(prod_tid[j], NULL);
    }
    printf("producer threads joined\n");

    return 0;
}



void *producer(void *ThreadPool){ //note that each thread should stay open to execute further files in real prog
	
	FILE *fo = (FILE *)open_file; //get arguments
	Thread_Pool Pool1 = (Thread_Pool *)ThreadPool; //arg2
	
	char f_line[MAXLINELENGTH];
	
	if(fo == NULL){
		printf("file not open!\n");
		pthread_exit(0);
	}
	int bc = 0;
	int bits = 0;
	int init = 1;
	char *p;
	while((p = fgets(f_line, MAXLINELENGTH, fo)) != NULL){

		pthread_mutex_lock(&mutex_sh);
		char *hostname = malloc(sizeof(p));
		pthread_mutex_unlock(&mutex_sh);

		int i=0;
		while((i < MAXLINELENGTH) && (p[i] != '\n')){
			hostname[i] = p[i];
			i++;
		}

		printf("%s\n", hostname);

		pthread_mutex_lock(&mutex_sh);
		int ccount = shb_count;
		
		if(ccount >= BOUND){
			printf("buffer full\n");
			pthread_mutex_unlock(&mutex_sh); //give up resource lock
			pthread_cond_broadcast(&consumer_condition); //wakeup consumers
			pthread_cond_wait(&producer_condition, &mutex_sh); //wait for producer condition to be signaled. 
		}
		
		wait_s(&S_shared_buff); 
		
		shared_buffer[in] = hostname;
		out = in
		in = (in + 1) % BOUND;

		printf("added to buffer\n");
		shb_count = shb_count + 1; 
		pthread_mutex_unlock(&mutex_sh);
		pthread_cond_signal(&consumer_condition);
		


		bits = bc;
		init = 0;
	}
	pthread_mutex_lock(&mutex_sh);
	//producers done
	producers_done = 1;
	pthread_mutex_unlock(&mutex_sh);
	//just because you serviced a file does not mean you can kill the tread, thread syntax should be while() loop not 1 call. 

	pthread_exit(0);
}


void *consumer(){
	char *host;
	while(1){
			pthread_mutex_lock(&mutex_sh);
			int ccount = shb_count;
			if(ccount <= 0){
				printf("buffer empty\n");
				pthread_mutex_unlock(&mutex_sh); //release lock
				pthread_cond_broadcast(&producer_condition); //wakeup all producers, the buffer is empty!
				pthread_cond_wait(&consumer_condition, &mutex_sh);
			}
			//critical section //will either have mutex lock or have it from cond_wait
			out = (in - 1) % BOUND;
			char *tmp = shared_buffer[out];
			
			shared_buffer[out] = NULL;
			//printf("critical section from consumers\n"); //write resolved hostname to file results.txt
			char *fn = "/home/user/3753/results.txt";
			FILE* fo = fopen(fn, WRITE_MODE);

			if(fo == NULL){
				printf("error, file not open!\n");
				pthread_exit(0);
			}
			if(tmp != NULL){
				printf("hello from consumer!: \n 	resolving tmp = %s to IP address\n", tmp);
				pthread_mutex_lock(&consumer_write);
				size_t bytes_written = fwrite(tmp, 1, sizeof(tmp), fo);
				pthread_mutex_unlock(&consumer_write);
			}else{
				//case where tries to get null pointer from the buffer for some reason. 
			}
			

			in = out;
			shb_count = shb_count - 1;
			signal_s(&S_shared_buff);
			pthread_mutex_unlock(&mutex_sh);
			pthread_cond_signal(&producer_condition);
			
	}


	pthread_exit(0);
}
