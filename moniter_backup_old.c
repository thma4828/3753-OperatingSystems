/*void add_item(int k, state thrds[BOUND], struct moniter *self){
	pthread_mutex_lock(&mutex);
	thrds[k] = hungry;
	self->test(k, thrds, self);
	if(thrds[k] == eating){
		/*add item to buffer 
		pthread_mutex_unlock(&mutex);
		return;
	}else{
		pthread_mutex_unlock(&mutex);
		self->wait_s();
	}


}

void remove_item(int k, pthread_t thread_ID[BOUND], state thrds[BOUND], struct moniter *self){
	pthread_mutex_lock(&mutex);
	thrds[k] = hungry;
	self->test(k, thrds, self);
	if(thrds[k] == eating){
		/*remove item to buffer 
		pthread_mutex_unlock(&mutex);
		return;
	}else{
		pthread_mutex_unlock(&mutex);
		self->wait_s();
	}


}

void test(int k, state thrds[BOUND], struct moniter *self){
	pthread_mutex_lock(&mutex);
	for(int i=0; i<BOUND; i++){
		if(&thrds[i] != NULL && thrds[i] == eating){
			thrds[k] = hungry;
			pthread_mutex_unlock(&mutex);
			return;
		}
	}

	thrds[k] = eating; //set k = eating
	pthread_mutex_unlock(&mutex);
	
} 

void putdown(int k, pthread_t thread_ID[BOUND], state thrds[BOUND], struct moniter *self){
	pthread_mutex_lock(&mutex);
	thrds[k] = thinking; //set this thread to thinking. 
	self->broadcast_s(); //wakeup all blocked threads. 
	pthread_mutex_unlock(&mutex);
} */


/*void init(state thrds[BOUND], int counter){
	pthread_mutex_lock(&mutex);
	counter = BOUND;
	for(int i=0; i<BOUND; i++){
		thrds[i] = thinking;
	}
	pthread_mutex_unlock(&mutex);
}*/