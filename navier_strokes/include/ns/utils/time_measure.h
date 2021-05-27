#ifndef _NS_UTILS_TIME_MEASURE_H
#define _NS_UTILS_TIME_MEASURE_H

// Data wrapper (opaque)
typedef struct time_meas time_meas;

/**
 * Since it's hidden, the meas_time struct has to be 
 * initialized in time_measure.c
 *
 * @return The struct with both start and end timestamps
 */
time_meas *createTime();

/**
 * Starts recording the computing time and writes it
 * to the variable start
 *
 * @param now The actual time struct
 */
void startTime(time_meas *now);

/**
 * Stops recording the computing time and writes it
 * to the variable end
 *
 * @param now The actual time struct
 */
void stopTime(time_meas *now);

/**
 * Prints the current computation time
 * 
 * @param now The actual time struct
 */
void printTime(time_meas *now);

#endif
