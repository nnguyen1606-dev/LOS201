#ifndef PROCESSOR_THREAD_H
#define PROCESSOR_THREAD_H

#include <pthread.h>
#include <stdatomic.h>
#include <mqueue.h>

#include "circular_buffer.h"
#include "logger_thread.h"

typedef struct {
    CircularBuffer *buffer;
SharedSensorData *shared_data;
    mqd_t alert_queue;
mqd_t log_queue;    
_Atomic int *running;
    _Atomic int *logger_alive;
    _Atomic int *alert_alive;

    float temp_threshold;
    float humidity_threshold;
} ProcessorThreadArgs;

void *processor_thread_func(void *arg);

#endif
