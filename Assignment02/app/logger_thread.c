#include "include/logger_thread.h"
#include "include/sms_messages.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>

#define LOG_LIMIT (1024L * 1024L)

static const char *alert_name(AlertType type)
{
    switch (type) {
        case TEMP_HIGH: return "TEMP_HIGH";
        case HUMID_HIGH: return "HUMID_HIGH";
        case TEMP_SPIKE: return "TEMP_SPIKE";
        default: return "UNKNOWN";
    }
}

void *logger_thread_func(void *arg)
{
    LoggerThreadArgs *args = arg;
    if (!args || !args->running || !args->log_path) return NULL;

    FILE *fp = fopen(args->log_path, "a");

    if (!fp) {
        perror("Logger fopen");
        if (args->logger_alive)
            atomic_store(args->logger_alive, 0);
        return NULL;
    }

    if (args->logger_alive)
        atomic_store(args->logger_alive, 1);

    struct timespec last_flush;
    clock_gettime(CLOCK_MONOTONIC, &last_flush);

    while (1) {
        struct mq_attr state;
        if (!atomic_load(args->running) &&
            (mq_getattr(args->log_queue, &state) != 0 || state.mq_curmsgs == 0))
            break;

        char buffer[128];
        struct timespec deadline;
        if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) break;
        deadline.tv_nsec += 100000000L;
        if (deadline.tv_nsec >= 1000000000L) {
            deadline.tv_sec++;
            deadline.tv_nsec -= 1000000000L;
        }

        ssize_t n = mq_timedreceive(args->log_queue, buffer, sizeof(buffer), NULL, &deadline);
        if (n == (ssize_t)sizeof(LogMessage)) {
            LogMessage msg;
            memcpy(&msg, buffer, sizeof(msg));
            if (msg.type == LOG_SAMPLE) {
                fprintf(fp, "[SAMPLE][%llu] TEMP=%.2f HUMID=%.2f AVG_TEMP=%.2f AVG_HUMID=%.2f\n",
                        (unsigned long long)msg.timestamp_ms, msg.temperature, msg.humidity,
                        msg.avg_temperature, msg.avg_humidity);
            } else if (msg.type == LOG_ALERT) {
                fprintf(fp, "[ALERT][%llu] TYPE=%s VALUE=%.2f THRESHOLD=%.2f\n",
                        (unsigned long long)msg.timestamp_ms, alert_name(msg.alert_type),
                        msg.alert_value, msg.alert_threshold);
            }
            long position = ftell(fp);

            if (position > LOG_LIMIT) {
                char backup[512];

                int written = snprintf(
                    backup,
                    sizeof(backup),
                    "%s.1",
                    args->log_path
                );

                if (written < 0 ||
                    (size_t)written >= sizeof(backup)) {
                    fprintf(stderr, "Logger backup path too long\n");
                    break;
                }

                fflush(fp);
                fclose(fp);

                remove(backup);

                if (rename(args->log_path, backup) != 0)
                    perror("Logger rotate");

                fp = fopen(args->log_path, "a");

                if (!fp) {
                    perror("Logger reopen");
                    if (args->logger_alive)
                        atomic_store(args->logger_alive, 0);
                    return NULL;
                }
            }
        } else if (n < 0 && errno != ETIMEDOUT && errno != EINTR) {
            perror("Logger mq_timedreceive");
            break;
        }

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (now.tv_sec - last_flush.tv_sec >= 5) {
            fflush(fp);
            last_flush = now;
        }
    }
    fflush(fp);
    fclose(fp);

    if (args->logger_alive)
        atomic_store(args->logger_alive, 0);

    return NULL;
}
