# LOS201 - Embedded Linux Labs

## Student Information

- **Name:** Nguyen Tran Nam Nguyen
- **Student ID:** SE205174
- **Group:** 10

## Course

Embedded Linux Systems (LOS201)

## Lab Assignments

| Lab | Description | Status |
|-----|-------------|--------|
| [LAB01](LAB01/) | Linux Kernel, U-Boot, BusyBox, QEMU | Completed |
| [LAB02](LAB02/) | Device Driver and Filesystem Management | Completed |
| LAB03 | Coming soon | Pending |

---

## LAB01 - Embedded Linux System

### Technologies

- Linux Kernel 5.15
- U-Boot 2022.04
- BusyBox 1.35.0
- ARM Cortex-A9
- QEMU vexpress-a9

### Description

Build and configure an Embedded Linux system using Linux Kernel,
U-Boot, BusyBox, and QEMU.

### Files

- [LAB01 Source and Output](LAB01/)
- [LAB01 Report](LAB01/bao_cao/)

---

## LAB02 - Device Driver and Filesystem Management

### Technologies

- Linux Kernel 5.15
- ARM Cortex-A9
- QEMU vexpress-a9
- Linux Character Device Driver
- procfs and sysfs
- MTD and JFFS2
- BusyBox init

### Part A - Character Device Driver

Develop a Linux kernel module named `lab2_driver.ko`.

Features:

- Register a character device with major number 240.
- Create `/dev/lab2`.
- Implement open, read, write, and release operations.
- Exchange data between user space and kernel space.

### Part B - procfs and sysfs

Provide driver information through:

- `/proc/lab2_info`
- `/sys/class/lab2_class/lab2/status`

Display device information such as:

- Major number
- Open count
- Buffer length

### Part C - MTD and JFFS2

- Simulate NAND flash using nandsim.
- Create and mount a JFFS2 filesystem.
- Test file persistence after unmounting and remounting.

### Part D - BusyBox Init

- Configure BusyBox init scripts.
- Automatically load the kernel module during boot.
- Automatically create the device node.
- Test the driver in QEMU.

### Results

- Kernel module successfully compiled for ARM.
- Character device successfully tested.
- procfs and sysfs interfaces verified.
- Module loading and unloading verified.
- MTD/JFFS2 filesystem tested.
- BusyBox initialization tested.

### Files

- [LAB02 Project](LAB02/)
- [LAB02 Driver Source](LAB02/driver/)
- [LAB02 Report and Screenshots](LAB02/bao_cao/)

---

## Repository Structure

```text
LOS201/
├── LAB01/
│   ├── bao_cao/
│   ├── configs/
│   ├── output/
│   └── rootfs/
│
├── LAB02/
│   ├── bao_cao/
│   ├── configs/
│   ├── driver/
│   ├── mtd/
│   ├── output/
│   └── rootfs/
│
└── README.md
```

## Author

**Nguyen Tran Nam Nguyen**

Student ID: SE205174

Course: Embedded Linux Systems (LOS201)# LOS201 - Embedded Linux Labs

## Student Information
- Name: Nguyen Tran Nam Nguyen
- Student ID: SE205174
- Group: 10

## Course
Embedded Linux Systems

## Lab Assignments

| Lab | Description | Status |
|-----|-------------|--------|
| LAB01 | Linux Kernel, U-Boot, BusyBox, QEMU | Completed |
| LAB02 | Coming soon | Pending |
| LAB03 | Coming soon | Pending |

## LAB01
- Linux Kernel 5.15
- U-Boot 2022.04
- BusyBox 1.35.0
- ARM Cortex-A9
- QEMU vexpress-a9
