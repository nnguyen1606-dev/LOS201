#include "include/alert_thread.h"
#include "include/sms_messages.h"

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>
#include <string.h>

static const char *alert_type_name(AlertType type)
{
    switch (type) {
        case TEMP_HIGH:
            return "TEMP_HIGH";
        case HUMID_HIGH:
            return "HUMID_HIGH";
        case TEMP_SPIKE:
            return "TEMP_SPIKE";
        default:
            return "UNKNOWN";
    }
}

void *alert_thread_func(void *arg)
{
    AlertThreadArgs *args = (AlertThreadArgs *)arg;

    if (args == NULL || args->running == NULL)
        return NULL;

    if (args->alert_alive)
        atomic_store(args->alert_alive, 1);

    unsigned long temp_high_count = 0;
    unsigned long humid_high_count = 0;
    unsigned long temp_spike_count = 0;

    while (1) {
        struct mq_attr state;
        if (!atomic_load(args->running) &&
            (mq_getattr(args->alert_queue, &state) != 0 || state.mq_curmsgs == 0))
            break;
        char recv_buffer[128] = {0};
        AlertMessage alert = {0};

        struct timespec timeout;

        if (clock_gettime(CLOCK_REALTIME, &timeout) != 0)
            break;

        timeout.tv_nsec += 100000000L;

        if (timeout.tv_nsec >= 1000000000L) {
            timeout.tv_sec++;
            timeout.tv_nsec -= 1000000000L;
        }

        ssize_t received = mq_timedreceive(
            args->alert_queue,
            recv_buffer,
            sizeof(recv_buffer),
            NULL,
            &timeout
        );

        if (received < 0) {
            if (errno == ETIMEDOUT || errno == EINTR)
                continue;

            perror("Alert mq_timedreceive");
            break;
        }

        if ((size_t)received != sizeof(alert)) {
            fprintf(stderr, "Invalid alert message size\n");
            continue;
        }

        memcpy(&alert, recv_buffer, sizeof(alert));

        switch (alert.type) {
            case TEMP_HIGH:
                temp_high_count++;
                break;

            case HUMID_HIGH:
                humid_high_count++;
                break;

            case TEMP_SPIKE:
                temp_spike_count++;
                break;

            default:
                continue;
        }


fprintf(
    stderr,
    "[ALERT][%llu] TYPE: %s VALUE: %.2f THRESHOLD: %.2f\n",
    (unsigned long long)alert.timestamp_ms,
    alert_type_name(alert.type),
    alert.value,
    alert.threshold
);


        LogMessage log_msg = {0};

        log_msg.timestamp_ms = alert.timestamp_ms;
        log_msg.type = LOG_ALERT;
        log_msg.alert_type = alert.type;
        log_msg.alert_value = alert.value;
        log_msg.alert_threshold = alert.threshold;

        while (1) {
            if (args->logger_alive &&
                !atomic_load(args->logger_alive)) {
                fprintf(stderr, "Logger is not available\n");
                break;
            }

            struct timespec send_timeout;

            if (clock_gettime(CLOCK_REALTIME, &send_timeout) != 0) {
                perror("Alert clock_gettime");
                break;
            }

            send_timeout.tv_nsec += 100000000L;

            if (send_timeout.tv_nsec >= 1000000000L) {
                send_timeout.tv_sec++;
                send_timeout.tv_nsec -= 1000000000L;
            }

            if (mq_timedsend(
                    args->log_queue,
                    (const char *)&log_msg,
                    sizeof(log_msg),
                    0,
                    &send_timeout
                ) == 0) {
                break;
            }

            if (errno == ETIMEDOUT || errno == EINTR)
                continue;

            perror("Alert log queue");
            break;
        }
    }

    printf(
        "[ALERT STATS] TEMP_HIGH=%lu HUMID_HIGH=%lu TEMP_SPIKE=%lu\n",
        temp_high_count,
        humid_high_count,
        temp_spike_count
    );

    if (args->alert_alive)
        atomic_store(args->alert_alive, 0);

    return NULL;
}
