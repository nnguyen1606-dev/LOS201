#include "include/circular_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/////////
int circular_buffer_init(CircularBuffer *cb, size_t capacity)
{
    if (cb == NULL || capacity == 0)
        return -1;

    cb->data = calloc(capacity, sizeof(SensorData));

    if (cb->data == NULL)
        return -1;

    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->capacity = capacity;

    cb->total_produced = 0;
    cb->total_consumed = 0;
    cb->overflow_count = 0;
cb->shutdown = 0;


if (pthread_mutex_init(&cb->mutex, NULL) != 0) {
    free(cb->data);
    cb->data = NULL;
    return -1;
}


if (pthread_cond_init(&cb->not_empty, NULL) != 0) {
    pthread_mutex_destroy(&cb->mutex);
    free(cb->data);
    cb->data = NULL;
    return -1;
}

return 0;
}

int circular_buffer_push(CircularBuffer *cb, SensorData sample)
{
    if (cb == NULL || cb->data == NULL)
        return -1;

    pthread_mutex_lock(&cb->mutex);

if (cb->shutdown) {
    pthread_mutex_unlock(&cb->mutex);
    return -1;
}

if (cb->count == cb->capacity) {
    cb->overflow_count++;
    pthread_mutex_unlock(&cb->mutex);
    return -1;
}
    
cb->data[cb->tail] = sample;
cb->tail = (cb->tail + 1) % cb->capacity;
cb->count++;
cb->total_produced++;
pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->mutex);
    return 0;
}
int circular_buffer_pop(CircularBuffer *cb, SensorData *sample)
{
    if (cb == NULL || sample == NULL || cb->data == NULL)
        return -1;

    pthread_mutex_lock(&cb->mutex);

    /* Chờ khi buffer rỗng */

while (cb->count == 0 && !cb->shutdown) {
    pthread_cond_wait(&cb->not_empty, &cb->mutex);
}

if (cb->count == 0 && cb->shutdown) {
    pthread_mutex_unlock(&cb->mutex);
    return -1;
}
    /* Lấy dữ liệu tại vị trí head */

*sample = cb->data[cb->head];

    /* Cập nhật head, count và total_consumed */

cb->head = (cb->head + 1) % cb->capacity;
cb->count--;
cb->total_consumed++;

    pthread_mutex_unlock(&cb->mutex);

    return 0;
}

void circular_buffer_shutdown(CircularBuffer *cb)
{
    if (cb == NULL)
        return;

    pthread_mutex_lock(&cb->mutex);

    cb->shutdown = 1;

    pthread_cond_broadcast(&cb->not_empty);

    pthread_mutex_unlock(&cb->mutex);
}

void circular_buffer_destroy(CircularBuffer *cb)
{
    if (cb == NULL)
        return;

    /* Hủy condition variable */
pthread_cond_destroy(&cb->not_empty);

    /* Hủy mutex */
pthread_mutex_destroy(&cb->mutex);

    /* Giải phóng vùng nhớ */
free(cb->data);
    /* Đặt con trỏ data về NULL */
cb->data = NULL;
}    

/* Return circular buffer statistics safely */
int circular_buffer_get_stats(
    CircularBuffer *cb,
    CircularBufferStats *stats)
{
    if (cb == NULL || stats == NULL)
        return -1;

    if (pthread_mutex_lock(&cb->mutex) != 0)
        return -1;

    stats->current_size = cb->count;
    stats->total_produced = cb->total_produced;
    stats->total_consumed = cb->total_consumed;
    stats->overflow_count = cb->overflow_count;

    pthread_mutex_unlock(&cb->mutex);

    return 0;
}
