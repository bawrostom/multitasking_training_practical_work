#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include "acquisitionManager.h"
#include "msg.h"
#include "iSensor.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"






//producer count storage
volatile unsigned int produceCount = 0;
pthread_t producers[PRODUCER_COUNT];
//storage buffer
unsigned int buffer[256];
static void *produce(void *params);

/**
* Semaphores and Mutex
*/

//mutex
pthread_mutex_t mux;

//Semaphore
sem_t buffEmpty;
sem_t buffFull;
sem_t disp;


unsigned int check;
/*
* Creates the synchronization elements.
* @return ERROR_SUCCESS if the init is ok, ERROR_INIT otherwise
*/


static unsigned int createSynchronizationObjects(void);
/*
* Increments the produce count.
*/
static void incrementProducedCount(void);

static unsigned int createSynchronizationObjects(void)

{
	if (sem_init(&buffEmpty, 0, 1) == -1 || sem_init(&buffFull, 0, 0) == -1 || (sem_init(&disp, 0, 1) == -1)){
		perror("Semaphore not created");
		return ERROR_INIT;
		}
	pthread_mutex_init(&mux, NULL);
	printf("[acquisitionManager]Semaphore created\n");
	return ERROR_SUCCESS;
}

static void incrementProducedCount(void)
{
	produceCount++;
}

unsigned int getProducedCount(void)
{
	unsigned int p = 0;
	p = produceCount;
	return p;
}

//TODO create accessors to limit semaphore and mutex usage outside of this C module.

void mutex_lock(){
	pthread_mutex_lock(&mux);
}

void mutex_unlock(){
	pthread_mutex_unlock(&mux);
}


void sem_wait_buffEmpty(){
	sem_wait(&buffEmpty);
}

void sem_post_buffEmpty(){
	sem_post(&buffEmpty);
}

void sem_wait_buffFull(){
	sem_wait(&buffFull);
}

void sem_post_buffFull(){
	sem_post(&buffFull);
}

void sem_post_disp(){
	sem_post(&disp);
}

void sem_wait_disp(){
	sem_wait(&disp);
}



MSG_BLOCK getMessage(void){
	MSG_BLOCK m;
	m.checksum = 0;
	printf("getting message %d \n");
	sem_wait_buffFull();
	//mutex_lock();
	for (int i=0;i<DATA_SIZE;i++){
		m.mData[i] = buffer[i];
		m.checksum ^= m.mData[i];
	} 
	printf("Consumer id %d   Consuming message \n", gettid());
	//mutex_unlock();
	sem_post_buffEmpty();
	return m;
}





unsigned int acquisitionManagerInit(void)
{
	unsigned int i;
	printf("[acquisitionManager]Synchronization initialization in progress...\n");
	fflush( stdout );
	if (createSynchronizationObjects() == ERROR_INIT){
		perror("Failed to create semaphores");
		return ERROR_INIT;
	}
	
	printf("[acquisitionManager]Synchronization initialization done.\n");

	for (i = 0; i < PRODUCER_COUNT; i++)
	{
		if (pthread_create(&producers[i], NULL, &produce, NULL) !=0){
			perror("Failed on initializing producers");
		}
	}


	return ERROR_SUCCESS;
}

void acquisitionManagerJoin(void)
{
	unsigned int i;
	for (i = 0; i < PRODUCER_COUNT; i++)
	{
		if (pthread_join(producers[i], NULL) != 0){
			perror("Failed on join");
		}
	}

	sem_destroy(&buffFull);
	sem_destroy(&buffEmpty);
	sem_destroy(&disp);
	pthread_mutex_destroy(&mux);
	printf("[acquisitionManager]Semaphore cleaned\n");
}

void *produce(void* params)
{
	printf("[acquisitionManager]Producer created with id %d\n", gettid());
	unsigned int i = 0;
	MSG_BLOCK m;
	int r = 0;
	while (i < PRODUCER_LOOP_LIMIT)
	{
		i++;
		sleep(PRODUCER_SLEEP_TIME+(rand() % 5));
		sem_wait_buffEmpty();
		sem_wait_disp();
		//mutex_lock();

		//read the input message
		getInput(r,&m);

		//check the intigrity
		if (messageCheck(&m) == 0){
			printf("P1: Data corrupted! \n");
		}
		//copy to buffer
		else{
		for (int j=0;j<DATA_SIZE;j++){
			buffer[j]=m.mData[j];
			}
		incrementProducedCount();
		}
		printf("Producer %d produced message %d \n",gettid(), i);
		//mutex_unlock();
 		sem_post_buffFull();
	}

	printf("[acquisitionManager] %d termination\n", gettid());
	//TODO

}


