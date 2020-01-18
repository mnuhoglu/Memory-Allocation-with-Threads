#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 1000

struct node
{
	int id;
	int size;
};


queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]; // thread memory information
char  memory[MEMORY_SIZE]; // memory size



void release_function()
{
	//This function will be called
	//whenever the memory is no longer needed.
	//It will kill all the threads and deallocate all the data structures.
}

void my_malloc(int thread_id, int size)
{
	//This function will add the struct to the queue
	pthread_mutex_lock(&sharedLock);
	node thread;
	thread.id=thread_id;
	thread.size=size;
	myqueue.push(thread);
	pthread_mutex_unlock(&sharedLock);
}

void * server_function(void *)
{
	//This function should grant or decline a thread depending on memory size.

	int i =0;
	int index=0;
	bool check=true;
	while (check)
	{
		if(!myqueue.empty())
		{
			pthread_mutex_lock(&sharedLock);
			node first = myqueue.front();
			myqueue.pop();
			
			//requested thread will be unblocked
	
			if(first.size<MEMORY_SIZE-index)
			{
				thread_message[first.id]=index;
				index=index+first.size+1;
				i++;
			}
			else 
			{
				thread_message[first.id]=-1;
				i++;
			}
			sem_post(&semlist[first.id]);
			pthread_mutex_unlock(&sharedLock);
			
			if(i==5)
			check=false;
		}
		
	}
}

void * thread_function(void * id) 
{
	//This function will create a random size, and call my_malloc
	//Block
	//Then fill the memory with 1's or give an error prompt
	int* idthread = (int*) id;
	int size = rand() % 300+1;
	my_malloc(*idthread,size);
	sem_wait(&semlist[*idthread]);
	pthread_mutex_lock(&sharedLock);

	if(thread_message[*idthread]!=-1)
	{
		for(int i=thread_message[*idthread];i<=thread_message[*idthread]+size;i++)
		{
			memory[i]='1';
		}
	}
	else
		cout << "Thread " << *idthread << ": Not enough memory" << endl;
	pthread_mutex_unlock(&sharedLock);
}

void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
  	{char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



void dump_memory() 
{
 // You need to print the whole memory array here.
	cout << "Memory Dump: " << endl;
	for(int i=0;i<MEMORY_SIZE;i++)
		cout << memory[i] ;
}

int main (int argc, char *argv[])
 {
 	//You need to create a thread ID array here
	 int mythreads[NUM_THREADS];
	 for(int j=0;j<NUM_THREADS;j++)//this array holds the threads' ids
		 mythreads[j]=j;
	 init();
	 pthread_t thread[NUM_THREADS];//array of threads
	 for(int i=0;i<NUM_THREADS;i++)//threads created
		 pthread_create(&thread[i],NULL,&thread_function,(void*) &mythreads[i]);
	 
	 for(int k=0;k<NUM_THREADS;k++)//threads joined 
		 pthread_join(thread[k],NULL);
	 
	 pthread_join(server,NULL);
 
	 dump_memory(); // this will print out the memory
 	printf("\nMemory Indexes:\n" );
 	for (int i = 0; i < NUM_THREADS; i++)
 	{
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	}
 	printf("\nTerminating...\n");
 }