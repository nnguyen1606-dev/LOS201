#include "include/processor_thread.h"
#include "include/sms_messages.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>

static int send_message(mqd_t queue, const void *message,
                        size_t size, _Atomic int *logger_alive)
{
    while (1) {
        if (logger_alive && !atomic_load(logger_alive)) {
            errno = EPIPE;
            return -1;
        }

        struct timespec timeout;

        if (clock_gettime(CLOCK_REALTIME, &timeout) != 0)
            return -1;

        timeout.tv_nsec += 100000000L;

        if (timeout.tv_nsec >= 1000000000L) {
            timeout.tv_sec++;
            timeout.tv_nsec -= 1000000000L;
        }

        if (mq_timedsend(queue, (const char *)message,
                         size, 0, &timeout) == 0)
            return 0;

        if (errno == ETIMEDOUT || errno == EINTR)
            continue;

        return -1;
    }
}

void *processor_thread_func(void *arg)
{
    ProcessorThreadArgs *args = (ProcessorThreadArgs *)arg;

    if (args == NULL || args->buffer == NULL || args->running == NULL)
        return NULL;

    float temp_history[10] = {0};
    float humidity_history[10] = {0};

    unsigned int history_index = 0;
    unsigned int history_count = 0;

    float previous_temperature = 0.0f;
    int has_previous = 0;

    while (1) {
        SensorData sample;

        if (circular_buffer_pop(args->buffer, &sample) != 0)
            break;

        temp_history[history_index] = sample.temperature;
        humidity_history[history_index] = sample.humidity;

        history_index = (history_index + 1) % 10;

        if (history_count < 10)
            history_count++;

        float temp_sum = 0.0f;
        float humidity_sum = 0.0f;

        for (unsigned int i = 0; i < history_count; i++) {
            temp_sum += temp_history[i];
            humidity_sum += humidity_history[i];
        }

        float avg_temperature = temp_sum / history_count;
        float avg_humidity = humidity_sum / history_count;

        if (args->shared_data != NULL) {
            pthread_mutex_lock(&args->shared_data->mutex);
            args->shared_data->latest_sample = sample;
            args->shared_data->has_sample = 1;
            pthread_mutex_unlock(&args->shared_data->mutex);
        }

        LogMessage log_msg = {0};

        log_msg.timestamp_ms = sample.timestamp_ms;
        log_msg.type = LOG_SAMPLE;
        log_msg.temperature = sample.temperature;
        log_msg.humidity = sample.humidity;
        log_msg.avg_temperature = avg_temperature;
        log_msg.avg_humidity = avg_humidity;

        if (send_message(args->log_queue,
                         &log_msg,
                         sizeof(log_msg), args->logger_alive) != 0) {
            perror("Processor log queue");
        }

        int temp_high =
            sample.temperature > args->temp_threshold;

        int humid_high =
            sample.humidity > args->humidity_threshold;

        int temp_spike =
            has_previous &&
            fabsf(sample.temperature - previous_temperature) > 5.0f;

        AlertMessage alert = {0};
        alert.timestamp_ms = sample.timestamp_ms;

        if (temp_high) {
            alert.type = TEMP_HIGH;
            alert.value = sample.temperature;
            alert.threshold = args->temp_threshold;

            if (send_message(args->alert_queue,
                             &alert,
                             sizeof(alert), args->alert_alive) != 0) {
                perror("TEMP_HIGH");
            }
        }

        if (humid_high) {
            alert.type = HUMID_HIGH;
            alert.value = sample.humidity;
            alert.threshold = args->humidity_threshold;

            if (send_message(args->alert_queue,
                             &alert,
                             sizeof(alert), args->alert_alive) != 0) {
                perror("HUMID_HIGH");
            }
        }

        if (temp_spike) {
            alert.type = TEMP_SPIKE;
            alert.value =
                fabsf(sample.temperature - previous_temperature);
            alert.threshold = 5.0f;

            if (send_message(args->alert_queue,
                             &alert,
                             sizeof(alert), args->alert_alive) != 0) {
                perror("TEMP_SPIKE");
            }
        }

        previous_temperature = sample.temperature;
        has_previous = 1;
    }

    return NULL;
}
