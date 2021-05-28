#include "ns/utils/time_measurement.h"
#include <stdlib.h>
#include "ns/utils/logger.h"

// Data wrapper
typedef struct time_measurement_t {
    // Start time
    struct timeval start;
    // Stop time
    struct timeval stop;
} time_measurement_t;

time_measurement_t *time_measurement_create() {
    time_measurement_t *time = NULL;

    time = (time_measurement_t *) malloc(sizeof(time_measurement_t));
    if (time == NULL) return NULL;

    return time;
}

void time_measurement_start(time_measurement_t *time) {
    if (time == NULL) {
        log_error("Unable to set the time measurement start");
        return;
    };

    gettimeofday(&time->start, NULL);
}

void time_measurement_stop(time_measurement_t *time) {
    if (time == NULL) {
        log_error("`time` in time_measurement_stop is NULL");
        return;
    }

    gettimeofday(&time->stop, NULL);
}

time_t time_measurement_get_difference_microsecond(const time_measurement_t *const time) {
    if (time == NULL) return -1;

    return ((time->stop.tv_sec - time->start.tv_sec) * 1000000L + time->stop.tv_usec) - time->start.tv_usec;
}

void time_measurement_print_difference(const time_measurement_t *const time, const char *const text) {
    if (time == NULL) return;

    log_info("[TIME_MEASUREMENT]: %s -> %lldµs", text != NULL ? text : "",
             time_measurement_get_difference_microsecond(time));
}

time_measurement_t *time_measurement_create_and_start() {
    time_measurement_t *time = NULL;

    time = time_measurement_create();
    time_measurement_start(time);

    return time;
}

void time_measurement_stop_and_print(time_measurement_t *time, const char *const text) {
    if (time == NULL)return;

    time_measurement_stop(time);
    time_measurement_print_difference(time, text);
}
