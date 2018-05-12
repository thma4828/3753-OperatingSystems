README

1. my program uses only multi-lookup.c and not the header file (im sorry I ran out of time after implementing other features. )
2. My program takes a minimum of 5 argument, if you run with less than 5 arguments a usage message will display to list the order of the arguments. 
3. If less files are passed in than producers, the # of producers will be set to number of files. 
4. I synchronize my producers and consumers with a global mutex to provide monitor like functionality.

5. I have a max producers = 5, max consumers = 10 limit. 

BUILD INSTRUCTIONS:
	either use: a) gcc -pthread -Wall -Wextra -o myprog multi-lookup.c 
	or use 		b) the Makefile. 

ARGUMENT LIST;
	<num prod> <num cons> <res log> <req log> <file list> 
	I generally build my file by passing in each file name as additional char * to the list of arguments in the command line. 

NOTES:
	-builds with no warnings! deadlock and starvation free! 