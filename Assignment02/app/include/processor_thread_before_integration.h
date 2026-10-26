#ifndef PROCESSOR_THREAD_H
#define PROCESSOR_THREAD_H

#include <pthread.h>
#include <stdatomic.h>
#include <mqueue.h>

#include "circular_buffer.h"

typedef struct {
    CircularBuffer *buffer;
    mqd_t alert_queue;
    _Atomic int *running;

    float temp_threshold;
    float humidity_threshold;
} ProcessorThreadArgs;

void *processor_thread_func(void *arg);

#endif
