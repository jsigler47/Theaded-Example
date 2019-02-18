//To compile: gcc ThreadedExample.c -lpthread
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define T 3 //Num threads
#define N 10 //Num input
#define key 17 //Key for gen function.
int max = 0;
int min = 0;
int odd = 0;
int even = 0;
int sumArr[T] = {0};
int list[N] = {0};
int Gsum = 0;
int split = 0;
int leftover = 0;
pthread_barrier_t myBarr;

void *process(void* param);
int gen(int t, int i);
int findMin();
int findMax();
int findOdd();
int findEven();
int findSum(long s, int l);

void *process(void* param);

//-----------------------------------------------------------
//-----------------------------------------------------------
void *process( void* param ){ 
	//printf("Hello from thread %ld\n", (long)param);
	int i = (long)param;
		while(i < N){ //Generate integers for list.
			list[i] = gen(i % T, i);
			//printf("Thread %ld set list[%d] to %d\n", (long)param, i, list[i]);
			i += T;
		}

		//The list array must be filled before finding the statistics of it.
		//Barrier added to account for this. All threads must generate their
		//section of list before any thread starts calculations based on it.

		//printf("Waiting...\n");
		pthread_barrier_wait(&myBarr);
		//printf("Done waiting!\n");

		int j = (long)param * split; 		//starting point for partial sum 
		int limit = split;					//For 0 it's 0, for 1 it's T, for 2 it's 2T, etc.
		if((long)param == T - 1){	 
			limit += leftover; 		 		//Give the leftover work to the final thread.
		}
		//printf("Thread %ld is calling findSum with j: %d, and limit: %d\n", (long)param, j, limit);
		sumArr[(long)param] = findSum(j, limit);

		//Split work for the max, min, odd, and even functions between the first three threads.
		if((long)param == 0){
			max = findMax();
		}
		if((long)param == 1){
			min = findMin();
		}
		if((long)param == 2){
			odd = findOdd();
			even = findEven();
		}

	//Stop the thread after work is complete.
	pthread_exit( (void*) param ); 
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int gen(int t, int i){
	//Generate numbers for list. i++ to keep i from being 0.
	i++;
	return((t * 13)%key + (i * 7)%key + (t * i * key)%key);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int findMin(){
	//Find smallest int in list.
	int tmp = INT_MAX;
	for(int i = 0; i < N; i++){
		if(list[i] < tmp)
			tmp = list[i];
	}
	return tmp;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int findMax(){
	//Find largest int in list.
	int tmp = 0;
	for(int i = 0; i < N; i++){
		if(list[i] > tmp)
			tmp = list[i];
	}
	return tmp;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int findOdd(){
	//Count the number of odd ints in list.
	int tmp = 0;
	for(int i = 0; i < N; i++){
		if(list[i] % 2 == 1)
			tmp++;
	}
	return tmp;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int findEven(){
	//Count the number of even ints in list.
	int tmp = 0;
	for(int i = 0; i < N; i++){
		if(list[i] % 2 == 0)
			tmp++;
	}
	return tmp;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int findSum(long start, int length){ 
	//Find the sum of a subset of list from list[start] to list[start + limit].
	int tmp = 0;
	for(int i = start; i < length + start; i++){
		tmp += list[i];
		//printf("List[%d] = %d\n",i, list[i]);
	}
	//printf("Found sum from %ld to %ld to be %d.\n", start, start + length - 1, tmp);
	return tmp;
}
//-----------------------------------------------------------
//print is a function to format all output in one place.
//-----------------------------------------------------------
void print(){
	printf("----------------------------\n");
	for(int j = 0; j < N; j++){
		printf("list[%d] = %d\t", j, list[j]);
		if(j < T){
			printf("sum[%d] = %d", j, sumArr[j]);
		}
		printf("\n");
	}
	printf("----------------------------\n");
	printf("Max: %d\t Min: %d\n", max, min);
	printf("Odd: %d\t Even: %d\n", odd, even);
	printf("Global Sum: %d\n", Gsum);
	printf("----------------------------\n");
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int main(){

	pthread_t tid[T]; //store pthread IDs in array
	long i;
	pthread_barrier_init(&myBarr, NULL, T);

	//printf("In main: time to build threads\n");

	//Use split and leftover to determine how much extra work the last thread will have to take over.
	split = N / T;
	leftover = N - (split * T);
	//printf("Split: %d\nLefover: %d\n", split, leftover);
 
/*  //SEQUENTIAL LOGIC
	int sum = 0;
	for(i = 0; i < N; i++){
		list[i] = gen(i % 3, i);
	}
	for(i = 0; i < N; i++){
		printf("list[%ld] = %d\n", i, list[i]);
	}
	min = findMin();
	max = findMax();
	printf("Min is %d. Max is %d.\n", min, max);
	odd = findOdd();
	even = findEven();
	printf("Odd is %d. Even is %d.\n", odd, even);
	sum = findSum();
	printf("Global sum = %ld\n", sum);
*/

	for(i = 0; i < T; i++){
		//printf("Creating %ld\n", i);
		pthread_create(&tid[i], NULL, process, (void *)i); //type cast i to a void pointer and start the threads in the process function.
	}
	//printf("In main: all threads created\n");

	for(i = 0; i < T; i++){
		
		pthread_join(tid[i], NULL); //Waits on thread ID tid[i] to finish before moving on. 
		//printf(" Main: joined %ld\n", i);
	}

	for(i = 0; i < T; i++){
		//Calculate the global sum based on the partial sums provided by the threads.
		Gsum += sumArr[i];
	}

	print();
	
	pthread_barrier_destroy(&myBarr);

    return 0;
}