#include <stdio.h>
#define NUMP 6
#define NUMR 3

void add_resource(int a[6][4], int i, int j, int avail[3]){
	if(a[i][j] + 1 <= avail[j])
		a[i][j] = a[i][j] + 1;
}

void remove_resource(int a[6][4], int i, int j, int avail[3]){
	if(a[i][j] - 1 >= 0){
		a[i][j] = a[i][j] - 1;
		avail[j] = avail[j] + 1;
	}
}

int main(){
	int alloc[6][4]; //resource allocation of 6 threads with 3 resources (f1, f2, f3, buffer) f1 and f2 belong to producers
	/*col 1 = f1, col 2 = f2, col 3 = f3, col4 = buffer*/
	int available[4] = {1, 1, 1, 1}; //# avail resources
	int max[6][3] = {{1, 1, 1, 1} //p1
					{1, 1, 1, 1}
					{1, 1, 1, 1}
					{1, 1, 1, 1} //c1
					{1, 1, 1, 1}
					{1, 1, 1, 1}}




}