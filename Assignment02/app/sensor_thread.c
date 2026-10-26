#include "include/sensor_thread.h"
#include "../driver/sms_sensor.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include <sys/ioctl.h>
#include <time.h>
/* Task 4: Store 1000 jitter samples */
#define MAX_JITTER_SAMPLES 1000

static long jitter_samples[MAX_JITTER_SAMPLES];
static unsigned int jitter_count = 0;
/* Number of sampling deadlines missed before sleeping */
static unsigned long long missed_deadlines = 0;
/* Compare jitter values for sorting */
static int compare_jitter(const void *a, const void *b)
{
    long x = *(const long *)a;
    long y = *(const long *)b;

    if (x < y)
        return -1;

    if (x > y)
        return 1;

    return 0;
}

/* Task 4: Print jitter statistics and histogram */
static void print_jitter_statistics(int use_fifo)
{
    if (jitter_count == 0) {
        printf("[JITTER] No samples collected\n");
        return;
    }

    long sorted[MAX_JITTER_SAMPLES];
    long min = jitter_samples[0];
    long max = jitter_samples[0];

    long long sum = 0;

    unsigned int hist_0_100 = 0;
    unsigned int hist_100_500 = 0;
    unsigned int hist_500_1000 = 0;
    unsigned int hist_over_1000 = 0;

    for (unsigned int i = 0; i < jitter_count; i++) {

        long value = jitter_samples[i];

        sorted[i] = value;
        sum += value;

        if (value < min)
            min = value;

        if (value > max)
            max = value;

        if (value < 100)
            hist_0_100++;
        else if (value < 500)
            hist_100_500++;
        else if (value <= 1000)
            hist_500_1000++;
        else
            hist_over_1000++;
    }

    qsort(
        sorted,
        jitter_count,
        sizeof(long),
        compare_jitter
    );

    double avg = (double)sum / jitter_count;

    /* Nearest-rank method for the 99th percentile */
    unsigned int p99_index =
        (99U * jitter_count + 99U) / 100U - 1U;

    long p99 = sorted[p99_index];

    printf("\n========== SENSOR JITTER REPORT ==========\n");

    printf("[JITTER] Scheduling: %s\n",
           use_fifo ? "SCHED_FIFO" : "SCHED_OTHER");

    printf("[JITTER] Samples: %u/%u\n",
           jitter_count,
           MAX_JITTER_SAMPLES);

    printf("[JITTER] Min: %ld us\n", min);
    printf("[JITTER] Avg: %.2f us\n", avg);
    printf("[JITTER] Max: %ld us\n", max);
    printf("[JITTER] P99: %ld us\n", p99);
printf("[JITTER] Missed deadlines: %llu\n",
       missed_deadlines);

    printf("\n[JITTER HISTOGRAM]\n");

    printf("0-100 us:       %u\n", hist_0_100);
    printf("100-500 us:     %u\n", hist_100_500);
    printf("500-1000 us:    %u\n", hist_500_1000);
    printf(">1000 us:       %u\n", hist_over_1000);

    printf("==========================================\n\n");
}

/* Task 4: Sleep until the next valid absolute deadline */
static int sensor_sleep_until_next(
    struct timespec *next,
    unsigned int interval_ms)
{
    if (next == NULL || interval_ms == 0)
        return EINVAL;

    const long long NS_PER_SEC = 1000000000LL;
    const long long interval_ns =
        (long long)interval_ms * 1000000LL;

    /* Advance by one sampling period */
    long long deadline_ns =
        (long long)next->tv_sec * NS_PER_SEC
        + next->tv_nsec
        + interval_ns;

    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return errno;

    long long now_ns =
        (long long)now.tv_sec * NS_PER_SEC
        + now.tv_nsec;

    /*
     * If the deadline has already passed, record
     * the missed periods and move to a future deadline.
     */
    if (now_ns >= deadline_ns) {

        unsigned long long skipped =
            (unsigned long long)
            ((now_ns - deadline_ns) / interval_ns) + 1ULL;

        missed_deadlines += skipped;

        deadline_ns +=
            (long long)skipped * interval_ns;
    }

    /* Convert the next deadline to timespec */
    next->tv_sec =
        (time_t)(deadline_ns / NS_PER_SEC);

    next->tv_nsec =
        (long)(deadline_ns % NS_PER_SEC);

    int ret;

    do {
        ret = clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,
            next,
            NULL
        );
    } while (ret == EINTR);

    /* Measure actual wake-up jitter */
    if (ret == 0 && jitter_count < MAX_JITTER_SAMPLES) {

        struct timespec actual;

        if (clock_gettime(CLOCK_MONOTONIC, &actual) == 0) {

            long long actual_ns =
                (long long)actual.tv_sec * NS_PER_SEC
                + actual.tv_nsec;

            long long jitter_ns =
                actual_ns - deadline_ns;

            if (jitter_ns < 0)
                jitter_ns = 0;

            long jitter_us =
                (long)(jitter_ns / 1000LL);

            jitter_samples[jitter_count] = jitter_us;
            jitter_count++;
        }
    }

    return ret;
}

    /* Normalize nanoseconds */

void *sensor_thread_func(void *arg)
{
    SensorThreadArgs *args = (SensorThreadArgs *)arg;

    if (args == NULL || args->buffer == NULL || args->running == NULL)
        return NULL;

/* Task 4: Verify actual scheduling policy */
int actual_policy;
struct sched_param actual_param;

int sched_error = pthread_getschedparam(
    pthread_self(),
    &actual_policy,
    &actual_param
);

if (sched_error != 0) {
    fprintf(stderr,
            "[RT] pthread_getschedparam failed: %s\n",
            strerror(sched_error));
    return NULL;
}

printf("[RT VERIFY] Actual policy: %s\n",
       actual_policy == SCHED_FIFO ? "SCHED_FIFO" :
       actual_policy == SCHED_OTHER ? "SCHED_OTHER" :
       "OTHER_POLICY");

printf("[RT VERIFY] Actual priority: %d\n",
       actual_param.sched_priority);

/* Do not label a failed RT setup as a valid FIFO measurement */
if (args->use_fifo &&
    (actual_policy != SCHED_FIFO ||
     actual_param.sched_priority != args->rt_priority)) {

    fprintf(stderr,
            "[RT] Requested SCHED_FIFO priority %d "
            "but actual configuration differs\n",
            args->rt_priority);

    return NULL;
}

    const char *path = args->device_path;

if (path == NULL)
    path = SMS_DEVICE_PATH;

unsigned int interval = args->interval_ms ? args->interval_ms : 500;

/* Task 4: Initialize absolute sampling clock */
struct timespec next_wakeup;

int time_error = clock_gettime(
    CLOCK_MONOTONIC,
    &next_wakeup
);

if (time_error != 0) {
    perror("Sensor clock_gettime");
    return NULL;
}


#ifdef HOST_TEST

    /* Host mode: generate simulated sensor samples */
    unsigned int sample_index = 0;

    while (atomic_load(args->running)) {

        SensorData sample = {0};

        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);

        sample.timestamp_ms =
            (unsigned long long)now.tv_sec * 1000ULL
            + (unsigned long long)now.tv_nsec / 1000000ULL;

        sample.temperature = 20.0f + (float)(sample_index % 26);
        sample.humidity = 40.0f + (float)(sample_index % 51);

        if (circular_buffer_push(args->buffer, sample) != 0) {
            fprintf(stderr, "Failed to push simulated sensor data\n");
        }

        if (args->verbose) {
            printf("[HOST_TEST] Temperature: %.2f, Humidity: %.2f\n",
                   sample.temperature, sample.humidity);
        }

        sample_index++;
        int sleep_error = sensor_sleep_until_next(
    &next_wakeup,
    interval
);

if (sleep_error != 0) {
    fprintf(stderr,
            "Sensor clock_nanosleep: %s\n",
            strerror(sleep_error));
    break;
}
    }

#else
    int config_fd = open(path, O_RDONLY);

    if (config_fd < 0) {
        perror("Sensor config open");
        return NULL;
    }

    if (ioctl(config_fd, SMS_SET_INTERVAL, &interval) < 0) {
        perror("SMS_SET_INTERVAL");
        close(config_fd);
        return NULL;
    }

    close(config_fd);

    while (atomic_load(args->running)) {

        char read_buffer[128];

        int fd = open(path, O_RDONLY);

        if (fd < 0) {
            if (errno != EINTR)
                perror("Failed to open sensor device");

   int sleep_error = sensor_sleep_until_next(
    &next_wakeup,
    interval
);

if (sleep_error != 0) {
    fprintf(stderr,
            "Sensor clock_nanosleep: %s\n",
            strerror(sleep_error));
    break;
}         

           continue;
        }

        ssize_t bytes_read;

        do {
            bytes_read = read(fd, read_buffer,
                              sizeof(read_buffer) - 1);
        } while (bytes_read < 0 &&
                 errno == EINTR &&
                 atomic_load(args->running));

        int read_errno = errno;
        close(fd);

        if (bytes_read < 0) {
            if (read_errno != EINTR && read_errno != EAGAIN)
                fprintf(stderr, "Sensor read error: %s\n",
                        strerror(read_errno));

        } else if (bytes_read == 0) {

            if (args->verbose)
                printf("No sensor data available\n");

        } else {

            read_buffer[bytes_read] = '\0';

            if (args->verbose)
                printf("Sensor raw data: %s\n", read_buffer);

            SensorData sample = {0};

            unsigned long long timestamp = 0;

            int parsed = sscanf(
                read_buffer,
                "%llu,%f,%f",
                &timestamp,
                &sample.temperature,
                &sample.humidity
            );

            if (parsed == 3) {

                sample.timestamp_ms = timestamp;

                if (args->verbose) {
                    printf("Timestamp: %llu\n", timestamp);
                    printf("Temperature: %.2f\n",
                           sample.temperature);
                    printf("Humidity: %.2f\n",
                           sample.humidity);
                }

                if (circular_buffer_push(args->buffer, sample) == 0) {

                    if (args->verbose)
                        printf("Sensor data pushed to buffer\n");

                } else {
                    fprintf(stderr,
                            "Failed to push sensor data\n");
                }

            } else {
                fprintf(stderr, "Invalid sensor data\n");
            }
        }

        int sleep_error = sensor_sleep_until_next(
    &next_wakeup,
    interval
);

if (sleep_error != 0) {
    fprintf(stderr,
            "Sensor clock_nanosleep: %s\n",
            strerror(sleep_error));
    break;
}
    }
#endif

    /* Task 4: Print jitter results before Sensor Thread exits */
    print_jitter_statistics(args->use_fifo);

    return NULL;
}
