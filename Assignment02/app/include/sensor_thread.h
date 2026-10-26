#ifndef SENSOR_THREAD_H
#define SENSOR_THREAD_H
#include <stdatomic.h>
#include <pthread.h>
#include "circular_buffer.h"

typedef struct {
    CircularBuffer *buffer;
    const char *device_path;
    unsigned int interval_ms;
    int verbose;
    _Atomic int *running;

    /* Task 4: Real-Time configuration */
    int use_fifo;
    int rt_priority;

} SensorThreadArgs;

void *sensor_thread_func(void *arg);

#endif

