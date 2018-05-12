#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define BOUND 6
void *producer(void *param);
void *consumer();

int tids[BOUND];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex lock for monitor
pthread_cond_t thread_condition1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t thread_condition2 = PTHREAD_COND_INITIALIZER;


typedef struct moniter{
	
	char *buffer[BOUND];
	int counter;
	int in;
	int out;

	void (*add_item)(char *, char *[BOUND], struct moniter *); 
	void (*remove_item)();
	void (*init)();
	char * (*a_malloc)(int p);
	void (*wait_s)();
	void (*wait_s2)();
	void (*signal_s)();
	void (*broadcast_s)();
	void (*broadcast_s2)();
};

struct moniter Moniter_1 = {};

char *a_malloc(int p){
	pthread_mutex_lock(&mutex);
	char *m = malloc(p);
	pthread_mutex_unlock(&mutex);
	return m;
}

void add_item(char *item, char *buffer[BOUND], struct moniter *M){
	pthread_mutex_lock(&mutex);
	if(M->counter >= BOUND){
		pthread_mutex_unlock(&mutex); //release lock and then block
		M->wait_s(); //when wait is complete will have control of mutex again. 
	}
	printf("hello from producer: %s\n", item);
	buffer[M->in] = item; //finished blocking, now add item.
	M->in = (M->in + 1) % BOUND;
	M->counter++;
	M->broadcast_s2(); //wakup all consumers. 
	pthread_mutex_unlock(&mutex);
}

void remove_item(char *buffer[BOUND], struct moniter *M){
	pthread_mutex_lock(&mutex);
	if(M->counter <= 0){
		pthread_mutex_unlock(&mutex);
		M->wait_s2(); //block until signalled and will get control of mutex. 
	}
	char *b = buffer[M->out];
	if(b != NULL)
		printf("hello from consumer: %s\n", b);
	else
		printf("consumer tried to pull out null string\n");

	buffer[M->out] = NULL;
	M->out = (M->out + 1) % BOUND;
	M->broadcast_s(); //wakeup all producers. 
	pthread_mutex_unlock(&mutex);
}

void init(struct moniter *M){
	M->in = 0;
	M->out = 0;
	M->counter = 0;
}

void wait_s(){
	pthread_cond_wait(&thread_condition1, &mutex);
}

void wait_s2(){
	pthread_cond_wait(&thread_condition2, &mutex);
}

void signal_s(){
	pthread_cond_signal(&thread_condition1);
}

void broadcast_s(){
	pthread_cond_broadcast(&thread_condition1);
}

void broadcast_s2(){
	pthread_cond_broadcast(&thread_condition2);
}






int main(){
	int i1 = pthread_cond_init(&thread_condition1, NULL);


	//creating a thread pool for producers
    pthread_t prod_tid[BOUND];
    
    int tc = 0;

    //int fd = open("/PA3/input/names1.txt", O_RDONLY);
    char *a1 = "/home/user/3753/PA3/input/names1.txt";
    char *a2 = "/home/user/3753/PA3/input/names2.txt";
    char *a3 = "/home/user/3753/PA3/input/names3.txt";

    char **files = malloc(8 * 4);
    files[0] = a1;
    files[1] = a2;
    files[2] = a3;

    Moniter_1.init(&Moniter_1);

    for(int i=0; i<3; i++){
    	prod_tid[i] = i;
    	pthread_create(&prod_tid[i], NULL, producer, files[i]);
    	tids[tc] = (int)prod_tid[i];
    	tc++;
    	printf("%d\n", i);
    }
    printf("producer threads created\n");
    

    //creating a thread pool for consumers
    pthread_t con_tid[BOUND];
    
    for(int i=0; i<3; i++){
    	con_tid[i] = i;
    	pthread_create(&con_tid[i], NULL, consumer, NULL);
    	tids[tc] = (int)con_tid[i];
    	tc++;
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
    	pthread_join(prod_tid[j], NULL);
    }
    printf("producer threads joined\n");

	int i2 = pthread_cond_destroy(&thread_condition1);


	if(i1 == 0 && i2 == 0)
		printf("success\n");
	else
		printf("failure: codes = %d, %d\n", i1, i2);
    return 0;
}
int p_done = 0;

void *producer(void *param){ //note that each thread should stay open to execute further files in real prog
	
	FILE* fo = fopen((const char *)param, "r");
	
	char f_line[100];
	
	if(fo == NULL){
		printf("file not open!\n");
		pthread_exit(0);
	}
	int bc = 0;
	int bits = 0;
	int init = 1;
	char *p;
	while((p = fgets(f_line, 100, fo)) != NULL){
		//make atomic malloc
		char *hostname = Moniter_1.a_malloc((int)sizeof(p));
		int i=0;
		while((i < 100) && (p[i] != '\n')){
			hostname[i] = p[i];
			i++;
		}
		printf("%s\n", hostname);
		Moniter_1.add_item(hostname, Moniter_1.buffer, &Moniter_1);
	}
	p_done = 1;

}

void *consumer(){
	while(!p_done){
		Moniter_1.remove_item(Moniter_1.buffer, &Moniter_1);
	}
}	
		













