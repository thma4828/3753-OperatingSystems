#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define ERROR1 99


#define READ_MODE "r"
#define WRITE_MODE "w"
#define BOUND 15
#define MAX_CONS 3
#define MAX_PROD 3

#define MAX_ALLOCS 1000
#define MAXLINELENGTH 50
//list of blocked consumers/producers
//pthread_t blocked_consumers[MAX_CONS] = {0};
//pthread_t blocked_producers[MAX_PROD] = {0};
//counters
//unsigned int b_cons = 0;
//unsigned int b_prod = 0;

//mutex for produces
pthread_mutex_t mutex_sh = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t sema = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_write = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t producer_write = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t thread_condition;

int producers_done = 0;
int p1_done = 0;
int p2_done = 0;
int p3_done = 0;
int consumers_done = 0;
int buffer_full = 0;

int prod_done[MAX_PROD];
int prod_done_count = 0;
//mutex for consumers. 
pthread_mutex_t mutex_consumers = PTHREAD_MUTEX_INITIALIZER;

void *producer();
void *consumer();

char *shared_buffer[BOUND];
unsigned int in = 0;
unsigned int out = BOUND-1;
unsigned int shb_count = 0;

pthread_t *tail;
pthread_t *head;
pthread_t *min_head;
pthread_t *oldhead;

typedef struct{
	int value;
	pthread_t *id_array;
	int counter;
}semaphore;

semaphore S_shared_buff = {
	.value = BOUND,
	.id_array = NULL,
	.counter = 0,
};

typedef struct{ //track all pointers I assign to the heap so that I dont memory leak. 
	char *char_addr_on_heap[MAX_ALLOCS];
	pthread_t *pt_addr_on_heap[MAX_ALLOCS];
	int cc;
	int cp;
}heap_address_tracker;


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

//heap address tracker for memory management. 
pthread_mutex_t hatm = PTHREAD_MUTEX_INITIALIZER;
heap_address_tracker HAT = {
	.cc = 0,
	.cp = 0,
};

pthread_mutex_t blocked_manage = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t consumer_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer_condition = PTHREAD_COND_INITIALIZER;

int main(int argc, char **argv){
    //creating a thread pool for producers
    pthread_t prod_tid[10];

    //int fd = open("/PA3/input/names1.txt", O_RDONLY);
    char *a1 = "/home/user/3753/PA3/input/names1.txt";
    char *a2 = "/home/user/3753/PA3/input/names2.txt";
    char *a3 = "/home/user/3753/PA3/input/names3.txt";

    char **files = malloc(8 * 4);
    files[0] = a1;
    files[1] = a2;
    files[2] = a3;

    S_shared_buff.id_array = malloc(sizeof(pthread_t) * BOUND);
    HAT.pt_addr_on_heap[HAT.cp] = S_shared_buff.id_array;
    HAT.cp++;

    for(int i=0; i<3; i++){
    	prod_tid[i] = i;
    	pthread_create(&prod_tid[i], NULL, producer, files[i]);
    	printf("%d\n", i);
    }
    printf("producer threads created\n");
    

    //creating a thread pool for consumers
    pthread_t con_tid[10];
    
    for(int i=0; i<3; i++){
    	con_tid[i] = i;
    	pthread_create(&con_tid[i], NULL, consumer, NULL);
    	printf("%d\n", i);
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



void *producer(void *param){ //note that each thread should stay open to execute further files in real prog
	
	FILE* fo = fopen((const char *)param, READ_MODE);
	
	char f_line[MAXLINELENGTH];
	
	if(fo == NULL){
		printf("file not open!\n");
		pthread_exit(0);
	}
	int bc = 0;
	int bits = 0;
	int init = 1;
	char *p; 
	int fc = 0;
	while((p = fgets(f_line, MAXLINELENGTH, fo)) != NULL){
		pthread_mutex_lock(&mutex_sh);
		char *hostname = malloc(strlen(p));
		HAT.char_addr_on_heap[HAT.cc] = hostname;
		HAT.cc++;
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
			int r = pthread_cond_broadcast(&consumer_condition); //wakeup consumers
			if(r == EINTR)
				printf("error with signal");
			pthread_cond_wait(&producer_condition, &mutex_sh); //wait for producer condition to be signaled. 
		}
		/**deadlock hazard, also should use semaphore for producers to block if a buffer full*/
		//should be deadlocking on wait_s call
		wait_s(&S_shared_buff); //when this wait_S is called, it could be the case that the calling thread blocks with the mutex lock. 
		shared_buffer[in] = hostname;
		in = (in + 1) % BOUND;
		printf("added to buffer\n");
		shb_count = shb_count + 1; 
		pthread_mutex_unlock(&mutex_sh);
		pthread_cond_signal(&consumer_condition);
		


		bits = bc;
		init = 0;
	}
	fc++;
	pthread_mutex_lock(&mutex_sh);
	//producer done
	prod_done[prod_done_count] = 1;
	prod_done_count++;
	int sum = 0;
	for(int zz=0; zz<MAX_PROD; zz++){
		sum += prod_done[zz];
	}
	if(sum == MAX_PROD)
		producers_done = 1;
	pthread_mutex_unlock(&mutex_sh);
	FILE *serviced = fopen("home/user/3753/serviced.txt", "a");
	
	if(serviced){
		pthread_mutex_lock(&producer_write);
		fprintf(serviced, "Thread %lu serviced %d files", pthread_self(), fc);
		pthread_mutex_unlock(&producer_write);
	}else{
		printf("failed to write to serviced.txt\n");
	}
	
	//just because you serviced a file does not mean you can kill the tread, thread syntax should be while() loop not 1 call. 

	pthread_exit(0);
}


void *consumer(){
	char *host;
	while(1){
			pthread_mutex_lock(&mutex_sh);
			int ccount = shb_count;
			if(ccount <= 0){ //if buffer empty
				printf("buffer empty\n");
				pthread_mutex_unlock(&mutex_sh); //release lock
				if(producers_done)
					break;
				pthread_cond_broadcast(&producer_condition); //wakeup all producers, the buffer is empty!
				pthread_cond_wait(&consumer_condition, &mutex_sh); //block on consumer condition
			}
			//critical section //will either have mutex lock or have it from cond_wait
			out = (in - 1) % BOUND;
			char *tmp = shared_buffer[out];
			
			shared_buffer[out] = NULL;
			//printf("critical section from consumers\n"); //write resolved hostname to file results.txt
			char *fn = "/home/user/3753/results.txt";
			FILE* fo = fopen(fn, "a");

			if(fo == NULL){
				printf("error, file not open!\n");
				pthread_exit(0);
			}
			if(tmp != NULL){
				printf("hello from consumer!: \n 	resolving tmp = %s to IP address\n", tmp);
				struct hostent *host_struct = gethostbyname(tmp);
				if(!host_struct){
					printf("failed to resolve host name.\n");
				}else{
					char **addr_list = host_struct->h_addr_list;
					if(addr_list == NULL){
						printf("failed to resolve host name: %s \n", tmp);
					}else{
						if(addr_list[0]){
							char *current_addr = addr_list[0]; //4 byte IP address. 
							pthread_mutex_lock(&consumer_write); //get consumer write lock
							fprintf(fo, "hostname: %s address:  %02x\n", tmp, *current_addr);
							pthread_mutex_unlock(&consumer_write); //release the write lock. 
						}else{
							printf("failed to resolve host name: %s \n", tmp);
						}
					}
				}
			}else{
				//case where tries to get null pointer from the buffer for some reason. 
				//shb_count++; <-- this should maybe be added? 
			}
			
			in = out;
			shb_count = shb_count - 1;
			pthread_mutex_unlock(&mutex_sh);
			signal_s(&S_shared_buff);
			pthread_cond_signal(&producer_condition);
			
	}


	pthread_exit(0);
}


///* struct hostent {
  // char *h_name;       /* official name of host */
   //char **h_aliases;   /* alias list */
  // int h_addrtype;     /* host address type */
   //int h_length;       /* length of address */
   //char **h_addr_list; /* list of addresses */
//};
 