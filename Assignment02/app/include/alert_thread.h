#ifndef ALERT_THREAD_H
#define ALERT_THREAD_H

#include <pthread.h>
#include <stdatomic.h>
#include <mqueue.h>

typedef struct {
    mqd_t alert_queue;
    mqd_t log_queue;
    _Atomic int *running;
    _Atomic int *logger_alive;
    _Atomic int *alert_alive;
} AlertThreadArgs;

void *alert_thread_func(void *arg);

#endif

