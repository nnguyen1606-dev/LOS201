#include "../driver/sms_sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>

#include <stdatomic.h>
#include <mqueue.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "include/circular_buffer.h"
#include "include/sensor_thread.h"
#include "include/processor_thread.h"
#include "include/alert_thread.h"
#include "include/logger_thread.h"
#include <time.h>
#include <math.h>
#define BUFFER_SIZE 10
#define QUEUE_NAME "/sms_alert_queue"
#define LOG_QUEUE_NAME "/sms_log_queue"
#define TEMP_THRESHOLD 40.0f
#define HUMIDITY_THRESHOLD 80.0f
#define SMS_LOG_PATH "/tmp/sms.log"
static _Atomic int running = 1;
static _Atomic int alert_running = 1;
static _Atomic int logger_running = 1;
static _Atomic int logger_alive = 0;
static _Atomic int alert_alive = 0;
static volatile sig_atomic_t stop_requested = 0;

/* Signal handler */
static void handle_signal(int sig)
{
    (void)sig;
    stop_requested = 1;
}
int main(int argc, char *argv[])
{
unsigned int interval_ms = 500;
float temp_threshold = TEMP_THRESHOLD;
float humidity_threshold = HUMIDITY_THRESHOLD;
unsigned int duration_sec = 60;
int verbose = 0;

int use_fifo = 0;
int rt_priority = 50;

int opt;


while ((opt = getopt(argc, argv, "i:t:h:d:vr")) != -1) {
    char *end;

    switch (opt) {
case 'r':
    use_fifo = 1;
    break;
        case 'i': {
            unsigned long value = strtoul(optarg, &end, 10);
            if (*optarg == '-' || *end != '\0' ||
                value == 0 || value > UINT_MAX) {
                fprintf(stderr, "Invalid sensor interval\n");
                return 1;
            }
            interval_ms = (unsigned int)value;
            break;
        }


case 't':
    errno = 0;
    temp_threshold = strtof(optarg, &end);

    if (end == optarg ||
        *end != '\0' ||
        errno == ERANGE ||
        !isfinite(temp_threshold)) {

        fprintf(stderr, "Invalid temperature threshold\n");
        return 1;
    }
    break;


case 'h':
    errno = 0;
    humidity_threshold = strtof(optarg, &end);

    if (end == optarg ||
        *end != '\0' ||
        errno == ERANGE ||
        !isfinite(humidity_threshold)) {

        fprintf(stderr, "Invalid humidity threshold\n");
        return 1;
    }
    break;


        case 'd': {
            unsigned long value = strtoul(optarg, &end, 10);
            if (*optarg == '-' || *end != '\0' ||
                value > UINT_MAX) {
                fprintf(stderr, "Invalid duration\n");
                return 1;
            }
            duration_sec = (unsigned int)value;
            break;
        }

        case 'v':
            verbose = 1;
            break;

        default:
            return 1;
    }
}
 
/* Task 4: Lock memory for real-time operation */
int memory_locked = 0;

if (use_fifo) {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == 0) {
        memory_locked = 1;
        printf("[RT] Memory locked successfully\n");
    } else {
        fprintf(stderr,
                "[RT] mlockall failed: %s\n",
                strerror(errno));
        fprintf(stderr,
                "[RT] Cannot guarantee locked memory\n");
    }
}
  
 CircularBuffer buffer;
    SharedSensorData shared = {0};

    pthread_t sensor_tid;
    pthread_t processor_tid;
    pthread_t alert_tid;
    pthread_t logger_tid;

    int sensor_started = 0;
    int processor_started = 0;
    int alert_started = 0;
    int logger_started = 0;

    int exit_status = 0;

    mqd_t queue = (mqd_t)-1;
mqd_t log_queue = (mqd_t)-1;
    struct mq_attr attr = {0};

    if (circular_buffer_init(&buffer, BUFFER_SIZE) != 0) {
        fprintf(stderr, "Circular buffer initialization failed\n");
        return 1;
    }

    if (pthread_mutex_init(&shared.mutex, NULL) != 0) {
        fprintf(stderr, "Shared mutex initialization failed\n");
        circular_buffer_destroy(&buffer);
        return 1;
    }

    /* Check sensor driver before starting threads */
#ifndef HOST_TEST    

int sensor_fd = open(SMS_DEVICE_PATH, O_RDONLY);
    if (sensor_fd < 0) {
        perror("Cannot open /dev/sms_sensor");
        pthread_mutex_destroy(&shared.mutex);
        circular_buffer_destroy(&buffer);
        return 1;
    }

    close(sensor_fd);

#endif

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 128;

    mq_unlink(QUEUE_NAME);

    queue = mq_open(
        QUEUE_NAME,
        O_CREAT | O_RDWR,
        0600,
        &attr
    );

    if (queue == (mqd_t)-1) {
        perror("mq_open");
        pthread_mutex_destroy(&shared.mutex);
        circular_buffer_destroy(&buffer);
        return 1;
    }
mq_unlink(LOG_QUEUE_NAME);

log_queue = mq_open(
    LOG_QUEUE_NAME,
    O_CREAT | O_RDWR,
    0600,
    &attr
);

if (log_queue == (mqd_t)-1) {
    perror("mq_open log_queue");
    exit_status = 1;
    goto cleanup;
}

    struct sigaction sa = {0};

    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) != 0 ||
        sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("sigaction");
        exit_status = 1;
        goto cleanup;
    }

    
SensorThreadArgs sensor_args = {
    .buffer = &buffer,
    .device_path = SMS_DEVICE_PATH,
    .interval_ms = interval_ms,
    .verbose = verbose,
    .running = &running,
    .use_fifo = use_fifo,
    .rt_priority = rt_priority
};

    ProcessorThreadArgs processor_args = {
        .buffer = &buffer,
        .shared_data = &shared,
        .alert_queue = queue,
.log_queue = log_queue,
        .running = &running,
        .logger_alive = &logger_alive,
        .alert_alive = &alert_alive,
.temp_threshold = temp_threshold,
.humidity_threshold = humidity_threshold
    };

    AlertThreadArgs alert_args = {
        .alert_queue = queue,
        .log_queue = log_queue,
        .running = &alert_running,
        .logger_alive = &logger_alive,
        .alert_alive = &alert_alive
    };

    LoggerThreadArgs logger_args = {
        .log_queue = log_queue,
        .shared_data = &shared,
.log_path = SMS_LOG_PATH,        
        .interval_ms = 1000,
        .running = &logger_running,
        .logger_alive = &logger_alive
    };

    printf("=================================\n");
if (verbose) {
    printf("[Config] Interval: %u ms\n", interval_ms);
    printf("[Config] Temperature threshold: %.2f C\n", temp_threshold);
    printf("[Config] Humidity threshold: %.2f %%\n", humidity_threshold);
    printf("[Config] Duration: %u seconds\n", duration_sec);
}    
printf(" Sensor Monitoring System Started\n");
    printf("=================================\n");

    /* Start Logger first */

/* Start Logger first */
int logger_create_error = pthread_create(
    &logger_tid,
    NULL,
    logger_thread_func,
    &logger_args
);

if (logger_create_error != 0) {
    fprintf(stderr,
            "Cannot create Logger Thread: %s\n",
            strerror(logger_create_error));

    exit_status = 1;
    goto stop_threads;
}

    logger_started = 1;

    /* Wait for Logger initialization */
    int logger_ready = 0;

    for (int i = 0; i < 50; i++) {
        if (atomic_load(&logger_alive)) {
            logger_ready = 1;
            break;
        }

        usleep(10000);
    }

    if (!logger_ready) {
        fprintf(stderr, "Logger initialization failed or timed out\n");
        exit_status = 1;
        goto stop_threads;
    }

    /* Start Alert before Processor */
    if (pthread_create(&alert_tid, NULL,
                       alert_thread_func, &alert_args) != 0) {
        fprintf(stderr, "Cannot create Alert Thread\n");
        exit_status = 1;
        goto stop_threads;
    }

    alert_started = 1;

    /* Wait for Alert Thread initialization */
    int alert_ready = 0;

    for (int i = 0; i < 50; i++) {
        if (atomic_load(&alert_alive)) {
            alert_ready = 1;
            break;
        }

        usleep(10000);
    }

    if (!alert_ready) {
        fprintf(stderr, "Alert initialization failed or timed out\n");
        exit_status = 1;
        goto stop_threads;
    }

    if (pthread_create(&processor_tid, NULL,
                       processor_thread_func, &processor_args) != 0) {
        fprintf(stderr, "Cannot create Processor Thread\n");
        exit_status = 1;
        goto stop_threads;
    }

    processor_started = 1;

    /* Start Sensor last */

/* Task 4: Configure Sensor Thread scheduling */
pthread_attr_t sensor_attr;
pthread_attr_t *sensor_attr_ptr = NULL;
int sensor_attr_initialized = 0;
int rt_error = 0;

if (use_fifo) {
    rt_error = pthread_attr_init(&sensor_attr);

    if (rt_error == 0) {
        sensor_attr_initialized = 1;

        rt_error = pthread_attr_setinheritsched(
            &sensor_attr,
            PTHREAD_EXPLICIT_SCHED
        );
    }

    if (rt_error == 0) {
        rt_error = pthread_attr_setschedpolicy(
            &sensor_attr,
            SCHED_FIFO
        );
    }

    if (rt_error == 0) {
        struct sched_param param = {0};
        param.sched_priority = rt_priority;

        rt_error = pthread_attr_setschedparam(
            &sensor_attr,
            &param
        );
    }

    if (rt_error != 0) {
        fprintf(stderr,
                "[RT] Cannot configure SCHED_FIFO: %s\n",
                strerror(rt_error));

        if (sensor_attr_initialized)
            pthread_attr_destroy(&sensor_attr);

        exit_status = 1;
        goto stop_threads;
    }

    sensor_attr_ptr = &sensor_attr;
}

/* Start Sensor last */
rt_error = pthread_create(
    &sensor_tid,
    sensor_attr_ptr,
    sensor_thread_func,
    &sensor_args
);

if (sensor_attr_initialized)
    pthread_attr_destroy(&sensor_attr);

if (rt_error != 0) {
    fprintf(stderr,
            "Cannot create Sensor Thread: %s\n",
            strerror(rt_error));

    exit_status = 1;
    goto stop_threads;
}

sensor_started = 1;

printf("[RT] Sensor scheduling: %s\n",
       use_fifo ? "SCHED_FIFO (priority 50)" : "SCHED_OTHER");

    printf("[Main] All four threads started\n");
    printf("[Main] Press Ctrl+C to stop\n");

struct timespec start_time, current_time;

if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
    perror("clock_gettime start");
    exit_status = 1;
    goto stop_threads;
}


while (!stop_requested) {

    if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0) {
        perror("clock_gettime current");
        exit_status = 1;
        break;
    }

    long elapsed = current_time.tv_sec - start_time.tv_sec;


    if (duration_sec > 0 && elapsed >= (long)duration_sec) {
        break;
    }

    sleep(1);
}

stop_threads:


    /* Stop Sensor Thread first */
    atomic_store(&running, 0);

    if (sensor_started)
        pthread_join(sensor_tid, NULL);

    /* Wake Processor and allow it to drain the buffer */
    circular_buffer_shutdown(&buffer);

    if (processor_started)
        pthread_join(processor_tid, NULL);

    /* Processor has finished sending alerts */
    atomic_store(&alert_running, 0);

    if (alert_started)
        pthread_join(alert_tid, NULL);

    /* Alert has finished sending log messages */
    atomic_store(&logger_running, 0);

    if (logger_started)
        pthread_join(logger_tid, NULL);

cleanup:

    if (queue != (mqd_t)-1) {
        mq_close(queue);
        mq_unlink(QUEUE_NAME);
    }

    if (log_queue != (mqd_t)-1) {
        mq_close(log_queue);
        mq_unlink(LOG_QUEUE_NAME);
    }

    pthread_mutex_destroy(&shared.mutex);
    circular_buffer_destroy(&buffer);

    printf("[Main] Sensor Monitoring System Stopped\n");

if (memory_locked) {
    if (munlockall() != 0) {
        perror("[RT] munlockall");
    }
}

    return exit_status;
}
