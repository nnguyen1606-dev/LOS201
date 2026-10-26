# LOS201 — Embedded Linux Systems

This repository contains laboratory exercises and assignments for **Embedded Linux Systems (LOS201)**, focusing on Linux kernel development, device drivers, embedded applications, debugging, performance profiling, and real-time scheduling.

The primary development environment uses **Ubuntu Linux** as the host system and **QEMU ARMv7-A Cortex-A9 (`vexpress-a9`)** as the target platform.

## Student Information

| Field | Information |
|---|---|
| Student | Nguyen Tran Nam Nguyen |
| Student ID | SE205174 |
| Group | 10 |
| Course | Embedded Linux Systems (LOS201) |
| Target Architecture | ARMv7-A Cortex-A9 |
| Kernel Version | Linux 5.15 |

## Repository Overview

| Project | Topic | Main Technologies | Directory |
|---|---|---|---|
| LAB01 | Embedded Linux Boot | Linux Kernel, U-Boot, BusyBox, QEMU | [LAB01](./LAB01/) |
| LAB02 | Device Drivers & Filesystems | Character Driver, procfs, sysfs, MTD, JFFS2 | [LAB02](./LAB02/) |
| LAB03 | Debugging & Performance Analysis | GDB, Valgrind, strace, perf, Real-Time Linux | [LAB03](./LAB03/) |
| Assignment 01 | Linux Device Driver Development | IOCTL, Misc Device, Mutex, Atomic Operations | [Assignment01](./Assignment01/) |
| Assignment 02 | Sensor Monitoring System | Kernel Module, POSIX Threads, Message Queues, Circular Buffer | [Assignment02](./Assignment02/) |

---

## LAB01 — Embedded Linux Boot

### Objective

Build a minimal ARM-based embedded Linux system and successfully boot it using QEMU.

### Main Features

- Configure and cross-compile the Linux kernel for ARMv7-A.
- Build and configure U-Boot.
- Compile BusyBox for an embedded root filesystem.
- Create an initramfs image.
- Configure kernel boot parameters.
- Boot the system using QEMU `vexpress-a9`.
- Verify kernel initialization and userspace startup.

### Technologies

`Linux 5.15` · `U-Boot` · `BusyBox` · `QEMU` · `ARM Cross-Compiler`

**Project:** [LAB01](./LAB01/)

---

## LAB02 — Device Drivers & Filesystem Management

### Objective

Develop Linux kernel modules and investigate embedded filesystem management.

### Main Features

- Implement a character device driver.
- Support `open()`, `read()`, `write()`, and `release()`.
- Exchange data between kernel space and userspace.
- Use `copy_to_user()` and `copy_from_user()`.
- Expose driver information through `/proc` and `/sys`.
- Load and unload kernel modules.
- Investigate MTD devices and NAND flash emulation.
- Work with JFFS2 and SquashFS images.
- Explore module initialization with BusyBox.

### Driver Interfaces

```text
/proc/lab2_info
/sys/class/lab2_class/lab2/status
```

### Technologies

`Linux Kernel Modules` · `Character Devices` · `procfs` · `sysfs` · `MTD` · `JFFS2`

**Project:** [LAB02](./LAB02/)

---

## LAB03 — Debugging & Performance Analysis

### Objective

Learn to debug embedded applications, detect memory errors, analyze system performance, and evaluate scheduling latency.

### Main Features

- Debug ARM applications using GDB and gdbserver.
- Identify and fix application defects.
- Detect memory leaks using Valgrind.
- Analyze system calls using strace.
- Profile application performance using perf.
- Evaluate scheduling latency.
- Compare standard Linux and real-time scheduling behavior.
- Generate performance logs and analysis reports.

### Tools

`GDB` · `gdbserver` · `Valgrind` · `strace` · `perf` · `cyclictest`

**Project:** [LAB03](./LAB03/)

---

## Assignment 01 — Linux Device Driver Development

### Objective

Develop and analyze Linux device drivers using IOCTL interfaces and the miscellaneous device framework.

### Task 1 — IOCTL Character Driver

The `asgn1_driver` module provides a character device:

```text
/dev/asgn1
```

Supported IOCTL commands:

| Command | Description |
|---|---|
| `ASGN1_RESET_BUFFER` | Reset the internal buffer and counters |
| `ASGN1_GET_STATS` | Retrieve driver statistics |
| `ASGN1_SET_MODE` | Configure driver operating mode |
| `ASGN1_GET_VERSION` | Retrieve the driver version |

Additional features include mutex synchronization, input validation, and kernel-to-userspace data transfer.

### Task 2 — Misc Counter Driver

The `counter_driver` module provides:

```text
/dev/counter
/proc/counter_info
```

Features:

- Register a miscellaneous device.
- Maintain a counter using `atomic_t`.
- Support counter read and write operations.
- Track read and write statistics.
- Validate userspace input.

### Tasks 3–4

- Driver architecture and implementation analysis.
- Technical documentation and theoretical questions.

### Assignment Files

- [Assignment 01 Directory](./Assignment01/)
- [IOCTL Driver](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/task1_ioctl/)
- [Misc Counter Driver](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/task2_misc/)
- [Test Logs](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/logs/)
- [PDF Report](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/bao_cao/SE205174_NguyenTranNamNguyen_Asgn01_BaoCao.pdf)

---

## Assignment 02 — Sensor Monitoring System

### Objective

Develop a multithreaded sensor monitoring application integrated with a Linux kernel device driver.

The system collects simulated sensor measurements, processes data, detects abnormal conditions, and records monitoring information.

### System Architecture

```text
+----------------------------+
| Linux Kernel Space         |
|                            |
| SMS Sensor Driver          |
| /dev/sms_sensor            |
| /proc/sms_stats            |
+-------------+--------------+
              |
              v
+----------------------------+
| User Space Application     |
|                            |
| Sensor Thread              |
|       |                    |
|       v                    |
| Circular Buffer            |
|       |                    |
|       v                    |
| Processor Thread           |
|       |                    |
|       v                    |
| POSIX Message Queues       |
|       |                    |
|       +----> Alert Thread  |
|       |                    |
|       +----> Logger Thread |
+----------------------------+
```

### Task 1 — Sensor Device Driver

Implement a Linux kernel module providing:

```text
/dev/sms_sensor
/proc/sms_stats
```

Main capabilities:

- Sensor data generation and reading.
- Character device operations.
- Configurable sampling interval using IOCTL.
- Driver statistics through procfs.
- ARM kernel module compilation.

### Task 2 — Multithreaded Monitoring Application

The application uses four main threads:

| Thread | Responsibility |
|---|---|
| Sensor Thread | Read or generate sensor samples |
| Processor Thread | Calculate averages and process samples |
| Alert Thread | Detect and display abnormal conditions |
| Logger Thread | Save sensor data and alerts |

Additional features:

- Circular buffer for sensor measurements.
- POSIX message queues for inter-thread communication.
- Temperature and humidity monitoring.
- Rolling-average calculations.
- Command-line configuration.
- Graceful shutdown and resource cleanup.

### Task 3 — Debugging and Performance Analysis

The application was tested using:

- **GDB:** Debugging and breakpoint verification.
- **Valgrind:** Memory leak and error detection.
- **strace:** System call tracing.
- **perf:** CPU performance profiling.
- **procfs:** Runtime statistics and memory information.

Recorded Valgrind results:

```text
Total heap usage: 8 allocs, 8 frees
In use at exit: 0 bytes
ERROR SUMMARY: 0 errors
```

### Task 4 — Scheduling and Jitter Analysis

Scheduling behavior was evaluated using:

- `SCHED_OTHER`
- `SCHED_FIFO`

Recorded measurements:

| Metric | SCHED_OTHER | SCHED_FIFO |
|---|---:|---:|
| Average jitter | 355.32 µs | 243.51 µs |
| P99 jitter | 460 µs | 312 µs |

The measured P99 jitter decreased by approximately **32.17%** in this test.

These values represent the recorded test conditions, not a universal real-time performance guarantee.

### Sensor Monitoring Test

The host application was compiled and executed successfully.

Example command:

```bash
./app/sms_app_x86 -i 500 -t 40 -h 80 -d 30
```

Recorded logger output:

```text
Sample records: 82
Alert records: 22
```

Verified alert types:

- `TEMP_HIGH`
- `HUMID_HIGH`
- `TEMP_SPIKE`

### Build Instructions

From the Assignment 02 directory:

```bash
cd Assignment02

make arm
make host
```

Generated applications:

```text
app/sms_app       ARMv7-A executable
app/sms_app_x86   x86-64 executable
```

The ARM build requires the appropriate cross-compilation toolchain.

Kernel module compilation additionally requires a configured and prepared Linux kernel build tree matching the target kernel.

### Assignment Files

- [Assignment 02 Directory](./Assignment02/)
- [Application Source Code](./Assignment02/app/)
- [Sensor Kernel Driver](./Assignment02/driver/)
- [Debugging and Performance Logs](./Assignment02/logs/)
- [Initramfs Output](./Assignment02/output/)
- [PDF Report](./Assignment02/bao_cao/SE205174_Asgn02_BaoCao.pdf)
- [Assignment 02 README](./Assignment02/README.md)

---

## Development Environment

| Component | Technology |
|---|---|
| Host Operating System | Ubuntu Linux |
| Target Architecture | ARMv7-A Cortex-A9 |
| Emulator | QEMU `vexpress-a9` |
| Linux Kernel | 5.15 |
| ARM Toolchain | `arm-linux-gnueabihf-` |
| Programming Languages | C, Shell |
| Build System | GNU Make |
| Debugging Tools | GDB, Valgrind, strace |
| Profiling Tools | perf |
| Concurrency | POSIX Threads, POSIX Message Queues |

---

## Repository Structure

```text
LOS201/
|
+-- LAB01/
|   +-- bao_cao/
|   +-- configs/
|   +-- output/
|   +-- rootfs/
|
+-- LAB02/
|   +-- bao_cao/
|   +-- configs/
|   +-- driver/
|   +-- mtd/
|   +-- output/
|   +-- rootfs/
|
+-- LAB03/
|   +-- apps/
|   +-- bao_cao/
|   +-- logs/
|   +-- output/
|
+-- Assignment01/
|   +-- SE205174_NguyenTranNamNguyen_Asgn01/
|   |   +-- bao_cao/
|   |   +-- logs/
|   |   +-- task1_ioctl/
|   |   +-- task2_misc/
|   +-- SE205174_NguyenTranNamNguyen_Asgn01.tar.gz
|
+-- Assignment02/
|   +-- app/
|   |   +-- include/
|   |   +-- *.c
|   |   +-- Makefile
|   |   +-- sms_app
|   |   +-- sms_app_x86
|   +-- driver/
|   |   +-- sms_sensor_driver.c
|   |   +-- sms_sensor.h
|   |   +-- sms_sensor.ko
|   |   +-- Makefile
|   +-- logs/
|   +-- output/
|   +-- bao_cao/
|   +-- Makefile
|   +-- README.md
|
+-- README.md
```

Empty directories are not tracked by Git.

---

## Reports

| Project | Report |
|---|---|
| LAB01 | Check [LAB01](./LAB01/) for available submission files |
| LAB02 | [PDF Report](./LAB02/bao_cao/SE205174_Lab02_BaoCao.pdf) |
| LAB03 | [PDF Report](./LAB03/bao_cao/SE205174_Lab03_BaoCao.pdf) |
| Assignment 01 | [PDF Report](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/bao_cao/SE205174_NguyenTranNamNguyen_Asgn01_BaoCao.pdf) |
| Assignment 02 | [PDF Report](./Assignment02/bao_cao/SE205174_Asgn02_BaoCao.pdf) |

---

## Notes

- ARM applications and kernel modules are intended for the configured embedded Linux target.
- Host test builds are provided where applicable.
- Performance measurements depend on the test environment and workload.
- Refer to individual project directories for source code, logs, build artifacts, and reports.
- Assignment 02 video demonstration is not included.

---

**Course:** Embedded Linux Systems (LOS201)  
**Student:** Nguyen Tran Nam Nguyen  
**Student ID:** SE205174  

