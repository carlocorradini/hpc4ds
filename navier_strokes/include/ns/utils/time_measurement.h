#ifndef _NS_UTILS_TIME_MEASUREMENT_H
#define _NS_UTILS_TIME_MEASUREMENT_H

#include <sys/time.h>

typedef struct time_measurement_t {
    // Start time
    struct timeval start;
    // Stop time
    struct timeval stop;
} time_measurement_t;

/**
 * Set the start time.
 *
 * @param time Time measurement struct reference
 */
void time_measurement_start(time_measurement_t *time);

/**
 * Set the stop time.
 *
 * @param time Time measurement struct reference
 */
void time_measurement_stop(time_measurement_t *time);

/**
 * Return the time difference in microsecond of time measurement `time`.
 *
 * @param time Time measurement struct reference
 * @return Time difference in microseconds, -1 otherwise
 */
time_t time_measurement_get_difference_microsecond(const time_measurement_t *time);

/**
 * Print using the logger the time difference.
 *
 * @param time Time measurement struct reference
 * @param text Optional text to append before the time difference.
 */
void time_measurement_print_difference(const time_measurement_t *time, const char *text);

/**
 * Utility function to execute two operations in one.
 * 1. Set the stop time.
 * 2. Print the time difference.
 *
 * @param time Time measurement struct reference
 * @param text Optional text to append before the time difference.
 */
void time_measurement_stop_and_print(time_measurement_t *time, const char *text);

#endif
