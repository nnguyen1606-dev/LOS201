#ifndef SMS_MESSAGES_H
#define SMS_MESSAGES_H

#include <stdint.h>

typedef enum {
    TEMP_HIGH = 1,
    HUMID_HIGH = 2,
    TEMP_SPIKE = 3
} AlertType;

typedef struct {
    uint64_t timestamp_ms;
    AlertType type;
    float value;
    float threshold;
} AlertMessage;

typedef enum {
    LOG_SAMPLE = 1,
    LOG_ALERT = 2
} LogType;

typedef struct {
    uint64_t timestamp_ms;
    LogType type;

    float temperature;
    float humidity;

    float avg_temperature;
    float avg_humidity;

    AlertType alert_type;
    float alert_value;
    float alert_threshold;
} LogMessage;

#endif
