/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
//#include "programs.c" 
#include "simulator.h"


void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    /* initialize static vars on first run */
    if(!initialized){
		for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
		    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
				timestamps[proctmp][pagetmp] = 0; //time stamp for each process in process list, and each page that belongs  to that process. 
		    }
		}
		initialized = 1;
    }

    /* Local vars */
    int proc;
    int pc;
    int page;
    int max_npages;
    int swapped = 0;
    int minpr, minpg;
     for(proc=0; proc<MAXPROCESSES; proc++) { 
		/* Is process active? */
		if(q[proc].active) {
		    /* Dedicate all work to first active process*/ 
		    pc = q[proc].pc; 		        // program counter for process
		    page = pc/PAGESIZE; 		// page the program counter needs
		    /* Is page swaped-out? */
		    max_npages = q[proc].npages;
		    if(page <= max_npages){
		    	timestamps[proctmp][pagetmp] = tick;
		    	if(!q[proc].pages[page]) {
					/* Try to swap in */
					if(!pagein(proc,page)) {
					    /* If swapping fails, swap out another page */
					   minpr = 0;
					   minpg = 0;
					   while(!swapped){
					   		for(int z=0; z<MAXPROCESSES; z++){
					   			for(int k=0; k<MAXPROCPAGES; k++){
					   				if(timestamps[z][k] < timestamps[minpr][minpg]){
					   					if(z != proc){
					   						minpr = z;
					   						minpg = k;
					   					}
					   				}
					   			}
					   		}
					   		if(pageout(minpr, minpg))
					   			swapped = 1;
					   }
					   break;
					}
		    	}

		    }
		    
		    /* Break loop after finding first active process */
		    break;
		}
    } 

    /* advance time for next pageit iteration */
    tick++;
} 
