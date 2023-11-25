#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h> 
#include <pthread.h>
#include "messageAdder.h"
#include "msg.h"
#include "iMessageAdder.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"

//consumer thread
pthread_t consumer;
//Message computed
volatile MSG_BLOCK out;
//In message
volatile MSG_BLOCK in;
//Consumer count storage
volatile unsigned int consumeCount = 0;

/**
 * Increments the consume count.
 */
static void incrementConsumeCount(void);

/**
 * Consumer entry point.
 */
static void *sum( void *parameters );


MSG_BLOCK getCurrentSum(){
	return out;
}

unsigned int getConsumedCount(){
	return consumeCount;
}

void incrementConsumeCount(void){
	consumeCount++;
}

void messageAdderInit(void){
	out.checksum = 0;
	for (size_t i = 0; i < DATA_SIZE; i++)
	{
		out.mData[i] = 0;
	}
	if (pthread_create(&consumer, NULL, &sum, NULL)!=0){
		perror("Failed to create consumer thread");
	}
}

void messageAdderJoin(void){
	if (pthread_join(consumer, NULL)!=0){
		perror("Failed on consumer thread join");
	}
}

static void *sum( void *parameters )
{
	printf("[messageAdder]Thread created for sum with id %d\n", gettid());
	unsigned int i = 0;
	while(i<ADDER_LOOP_LIMIT){
		i++;
		printf("Getting message \n");
		sleep(ADDER_SLEEP_TIME);
		in = getMessage();
		if (messageCheck(&in) == 0){
			printf("C: Data corrupted \n");
		}else{
			//at each 4 messages we reinitialize the accumulator
			if (i % 4 == 0){
				messageAdderInit();
				messageAdd(&out, &in);
			}else{
				messageAdd(&out, &in);
			}
			incrementConsumeCount();
		}
	}
	printf("[messageAdder] %d termination\n", gettid());
	//TODO
}


