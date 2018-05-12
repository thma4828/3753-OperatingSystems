#include <stdio.h>

void philosopher_deadlock(int i){
	while(1){
		wait(chopstick[i]);
		wait(chopstick[i+1 % CCOUNT]);

		//eat

		signal(chopstick[i]);
		signal(chopstick[i+1 % CCOUNT]);
	}
}
//THIS SOLUTION CAUSES DEADLOCK!, if all philosophers pick up their left chopstick they are in deadlocked state all waiting 
//on their neighbors == circular wait. 

struct monitor{
	enum{thinking, eating, hungry}state[5]; //state for each phil. 
	condition self[5]; //5 condition variables to block on, 1 for each philosopher

	void init(){
		for(int i=0; i<5; i++){
			state[i] = thinking;
		}
	}

	void pickup(int i){
		state[i] = hungry;
		test(i);
		if(state[i] != eating){
			self[i].wait(); //block phil.
		}
	}

	void test(int i){
		if(state[i-1 % 5] != eating && state[i+1 % 5] != eating && state[i] == hungry){
			state[i] = eating;
			self[i].signal(); //unblock this philosopher so it can eat
		}
	}

	void putdown(int i){
		state[i] = thinking; //release chopsticks
		test(i+1 % 5); //test both of its neighbors.
		test(i-1 % 5);

	}
}

void Philosopher(int i){
	monitor.pickup(i);
	//EAT
	monitor.putdown(i);


}