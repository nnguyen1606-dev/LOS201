# Embedded Linux Assignment 02
## Sensor Monitoring System (SMS)

### 1. Project Overview

This project implements a multithreaded Sensor Monitoring System
on Embedded Linux.

The system includes:

- Linux character device driver: /dev/sms_sensor
- Driver statistics interface: /proc/sms_stats
- Sensor Thread
- Processor Thread
- Alert Thread
- Logger Thread
- Thread-safe circular buffer
- POSIX message queues
- Real-time scheduling support
- Debugging and performance profiling

### 2. Project Structure

app/
    Application source code and Makefile

app/include/
    Application header files

driver/
    Linux kernel module source code and Makefile

kernel/linux-5.15/
    Linux kernel source

rootfs/
    Embedded Linux root filesystem

output/
    Kernel image, DTB, initramfs and compiled binaries

logs/
    Valgrind, strace, perf, GDB, memory and jitter results

report/
    Screenshots and report materials

### 3. Build Instructions

Build the ARM application:

    make arm

Build the x86 host application:

    make host

Build both applications:

    make all

Build the ARM kernel module:

    make driver

Clean application binaries:

    make clean

The ARM application is statically linked.

### 4. Run on Ubuntu Host

Build the host application:

    make host

Run for 60 seconds:

    ./app/sms_app_x86 -i 500 -t 40 -h 80 -d 60

Run continuously:

    ./app/sms_app_x86 -d 0

Stop continuous execution using Ctrl+C.

### 5. Run on QEMU ARM

Boot the Versatile Express Cortex-A9 machine:

    qemu-system-arm \
      -M vexpress-a9 \
      -m 256M \
      -kernel output/zImage \
      -dtb output/vexpress-v2p-ca9.dtb \
      -initrd output/initramfs_asgn2.cpio.gz \
      -append "console=ttyAMA0 rdinit=/init" \
      -nographic

Inside QEMU, verify the driver:

    cat /proc/sms_stats

Run the ARM monitoring application:

    /bin/sms_app -i 500 -t 40 -h 80 -d 60

Inspect log messages:

    cat /tmp/sms.log

### 6. Task 1: Sensor Monitoring System

The application includes four threads:

1. Sensor Thread:
   Reads sensor data and pushes samples into a circular buffer.

2. Processor Thread:
   Computes rolling averages and detects threshold violations.

3. Alert Thread:
   Receives and displays sensor alerts.

4. Logger Thread:
   Writes samples and alerts to /tmp/sms.log.

The application supports configurable sampling intervals,
thresholds, execution duration, and verbose output.

### 7. Task 2: Memory and Driver Integration

Valgrind result:

- Test duration: 35 seconds
- Memory remaining at exit: 0 bytes
- Memory errors: 0
- Graceful SIGINT shutdown: successful

Memory snapshots were collected at 5, 30, and 55 seconds.

Driver integration test:

- Sampling interval: 500 ms
- Actual read_count: 119
- Expected read_count: approximately 120

### 8. Task 3: Debugging and Performance

GDB:

- Verified TEMP_HIGH alert at 45 degrees Celsius.
- Confirmed Alert Thread receives the message.

strace:

- Test duration: 30 seconds
- Total system calls: 1283
- Exit status: 0

perf:

- Collected 838 CPU samples.
- Identified synchronization and message queue overhead.

### 9. Task 4: Real-Time Scheduling

The Sensor Thread supports:

- SCHED_FIFO priority 50
- SCHED_OTHER
- CLOCK_MONOTONIC
- TIMER_ABSTIME
- clock_nanosleep()
- mlockall(MCL_CURRENT | MCL_FUTURE)

Results:

Metric          SCHED_OTHER     SCHED_FIFO

Samples         1000            1000
Min jitter      62 us           10 us
Avg jitter      355.32 us       243.51 us
Max jitter      593 us          326 us
P99 jitter      460 us          312 us
Missed          0               0

P99 jitter decreased by approximately 32.17%
in the measured experiment.

### 10. Evidence Files

Task 1:

    logs/task1_final_stdout.txt
    logs/task1_final_stderr.txt

Task 2:

    logs/valgrind_sms_final.txt
    logs/memory_footprint.txt
    logs/proc_sms_stats_before.txt
    logs/proc_sms_stats_after.txt

Task 3:

    logs/gdb_sms.txt
    logs/strace_sms_final.txt
    logs/perf_sms.txt
    logs/perf.data

Task 4:

    logs/jitter_other_final.txt
    logs/jitter_fifo_final.txt

Screenshots are stored in the report/ directory.

### 11. Notes

Host mode uses simulated sensor data.

QEMU ARM mode communicates with the custom
Linux kernel character device driver.

CPU profiling and jitter results are environment-dependent.

The project was tested on Ubuntu and QEMU ARM
Versatile Express Cortex-A9.
