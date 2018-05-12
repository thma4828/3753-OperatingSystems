#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define MAXTHREADS 10
#define BOUND 15

#define PWB 0x11 //producer waiting buffer
#define CWB 0x22//consumer waiting buffer

#define PWF 0x33 //producer waiting file
#define CWF 0x44 //consumer waiting file

#define UB 0x55 //using buffer
#define PUF 0x66 //prod using file
#define CUF 0x66   //cons using file



typedef enum state{PWB, CWB, 
					PWF, CWF, 
					UB, PUF, CUF};

typedef struct moniter{
	char *buffer[BOUND]; ///the bounded buffer
	unsigned int condition[MAXTHREADS]; //condition 
	enum state[MAXTHREADS];
	unsigned int counter;
	unsigned int in;
	unsigned int out;
	pthread_mutex_t Mutex;

};

struct moniter Moniter1 = {};