#include "ns/utils/time_measure.h"
#include <sys/time.h>
#include <stdio.h>

// Data wrapper
typedef struct time_meas {
    // Two timestamps
    struct timeval start;
    struct timeval end;
} time_meas;

/**
 * Functions:
 */ 
time_meas *createTime() {
	time_meas *now = NULL;
    now = (time_meas *) malloc(sizeof(time_meas));
    
    if (now == NULL) {
    	return NULL;
    }

    return now;
}

void startTime(time_meas *now) {
    gettimeofday(&now->start, NULL);
}

void stopTime(time_meas *now) {
    gettimeofday(&now->end, NULL);
}

void printTime(time_meas *now) {
    printf("CHECK CHECK %ld\n", (
        		now->end.tv_sec*1000000 + now->end.tv_usec
      		  - now->start.tv_sec*1000000 - now->start.tv_usec));
}
