#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stddef.h>
#include <pthread.h>

typedef struct {
    unsigned long long timestamp_ms;
    float temperature;
    float humidity;
} SensorData;

typedef struct {
    SensorData *data;

    size_t head;
    size_t tail;
    size_t count;
    size_t capacity;
	int shutdown;

    unsigned long total_produced;
    unsigned long total_consumed;
    unsigned long overflow_count;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
} CircularBuffer;

int circular_buffer_init(CircularBuffer *cb, size_t capacity);

int circular_buffer_push(CircularBuffer *cb, SensorData sample);

int circular_buffer_pop(CircularBuffer *cb, SensorData *sample);

void circular_buffer_shutdown(CircularBuffer *cb);
void circular_buffer_destroy(CircularBuffer *cb);

/* Thread-safe circular buffer statistics */
typedef struct {
    size_t current_size;
    unsigned long total_produced;
    unsigned long total_consumed;
    unsigned long overflow_count;
} CircularBufferStats;

int circular_buffer_get_stats(
    CircularBuffer *cb,
    CircularBufferStats *stats
);

#endif
