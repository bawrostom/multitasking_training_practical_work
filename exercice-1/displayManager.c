#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "displayManager.h"
#include "iDisplay.h"
#include "iAcquisitionManager.h"
#include "iMessageAdder.h"
#include "msg.h"
#include "multitaskingAccumulator.h"
#include "debug.h"

// DisplayManager thread.
pthread_t displayThread;



/**
 * Display manager entry point.
 * */
static void *display( void *parameters );


void displayManagerInit(void){
	printf("Creating display thread \n");
	if (pthread_create(&displayThread, NULL ,&display, NULL)!=0){
		perror("Failed to init display thread");
	}
}

void displayManagerJoin(void){
	if (pthread_join(displayThread,NULL)!=0){
		perror("Failed to join 2 threads");
	}	
} 

static void *display( void *parameters )
{
	printf("[displayManager]Thread created for display with id %d\n", gettid());
	unsigned int diffCount = 1;
	unsigned int produced = 0;
	unsigned int consumed;
	unsigned int msg;
	MSG_BLOCK m;
	while(produced < DISPLAY_LOOP_LIMIT){
		sleep(DISPLAY_SLEEP_TIME);
		m = getCurrentSum();
		messageDisplay(&m);
		produced = getProducedCount();
		consumed = getConsumedCount();
		diffCount = produced - consumed;
		print(produced, consumed);
		sem_post_disp();
	}
	printf("[displayManager] %d termination\n", gettid());
   //TODO
}