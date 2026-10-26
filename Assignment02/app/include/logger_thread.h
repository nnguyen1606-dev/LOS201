#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

#include <pthread.h>
#include <stdatomic.h>
#include <mqueue.h>

#include "circular_buffer.h"

typedef struct {
    SensorData latest_sample;
    int has_sample;
    pthread_mutex_t mutex;
} SharedSensorData;

typedef struct {
    mqd_t log_queue;
    SharedSensorData *shared_data;
    const char *log_path;
    unsigned int interval_ms;
    _Atomic int *running;
    _Atomic int *logger_alive;
} LoggerThreadArgs;

void *logger_thread_func(void *arg);

#endif

