# LOS201 — Embedded Linux Systems

Repository lưu trữ bài thực hành và Assignment môn **Embedded Linux Systems (LOS201)** trên nền tảng **ARMv7-A Cortex-A9**, sử dụng Ubuntu làm host và QEMU `vexpress-a9` làm target.

## Student Information

| Field | Information |
|---|---|
| Student | Nguyen Tran Nam Nguyen |
| Student ID | SE205174 |
| Group | 10 |
| Course | Embedded Linux Systems (LOS201) |

## Lab & Assignment Overview

| Bài | Chủ đề theo đề bài | Nội dung trọng tâm | Thư mục |
|---|---|---|---|
| LAB01 | Cấu hình Kernel & Khởi động Hệ thống Linux Nhúng (ARMv7) | Kernel, U-Boot, BusyBox, initramfs, QEMU | [LAB01](./LAB01/) |
| LAB02 | Viết Device Driver & Quản lý File System | Character driver, procfs/sysfs, MTD/JFFS2, BusyBox init | [LAB02](./LAB02/) |
| LAB03 | Debugging Kernel & Phân tích Hiệu năng Ứng dụng Nhúng | GDB remote, Valgrind, strace/perf, cyclictest, PREEMPT_RT | [LAB03](./LAB03/) |
| Assignment 01 | How to Write a Device Driver | ioctl, miscdevice, driver analysis, theory | [Assignment01](./Assignment01/) |

## LAB01 — Kernel Configuration & Embedded Linux Boot

**Mục tiêu theo đề:** Xây dựng hệ thống Linux nhúng ARMv7 có thể khởi động trên QEMU.

- Cấu hình và cross-compile Linux kernel cho ARMv7 Cortex-A9.
- Làm việc với U-Boot và các tham số khởi động (`bootargs`, `bootcmd`).
- Xây dựng root filesystem tối giản bằng BusyBox, đóng gói `initramfs`.
- Khởi động kernel và userspace trong QEMU; kiểm tra boot log.

**Công cụ:** Linux kernel 5.15, U-Boot, BusyBox, ARM GNU toolchain, QEMU.

**Bài nộp:** [Mở thư mục LAB01](./LAB01/).

## LAB02 — Device Drivers & Filesystem Management

**Mục tiêu theo đề:** Phát triển kernel module và nghiên cứu giao diện driver/filesystem.

- Character device driver với `open`, `read`, `write`, `release`.
- Tương tác giữa user space và kernel space qua `copy_to_user()` và `copy_from_user()`.
- Cung cấp thông tin driver qua `/proc` và `/sys` (procfs/sysfs).
- Tìm hiểu MTD, mô phỏng NAND flash và filesystem JFFS2.
- Tìm hiểu cơ chế khởi động module bằng BusyBox init.

**Công cụ:** Linux kernel modules, QEMU, `mtd-utils`, BusyBox.

**Bài nộp:** [Mở thư mục LAB02](./LAB02/).

## LAB03 — Kernel Debugging & Performance Analysis

**Mục tiêu theo đề:** Phát hiện lỗi chương trình, phân tích hiệu năng và đánh giá độ trễ.

- **GDB multiarch + gdbserver:** remote debugging ứng dụng ARM trong QEMU; breakpoint, watchpoint và backtrace.
- **Valgrind:** kiểm tra memory leak và lỗi truy cập bộ nhớ.
- **strace / perf:** theo dõi system calls và phân tích CPU hotspots.
- **cyclictest / PREEMPT_RT:** đo và so sánh scheduling latency giữa các cấu hình kernel.

**Bài nộp:** [Mở thư mục LAB03](./LAB03/).

## Assignment 01 — How to Write a Device Driver

Assignment mở rộng kiến thức của LAB02, gồm bốn phần theo đề bài:

| Task | Nội dung | Điểm tối đa |
|---|---|---:|
| Task 1 | Mở rộng character driver với `ioctl` interface | 40 |
| Task 2 | Xây dựng misc counter device driver | 30 |
| Task 3 | Báo cáo phân tích và thiết kế driver | 20 |
| Task 4 | Trả lời câu hỏi lý thuyết | 10 |

### Task 1 — IOCTL Character Driver

Driver `asgn1_driver` cung cấp device `/dev/asgn1` và các lệnh điều khiển:

| IOCTL command | Chức năng |
|---|---|
| `ASGN1_RESET_BUFFER` | Xóa buffer và reset bộ đếm đọc/ghi |
| `ASGN1_GET_STATS` | Trả về thống kê hoạt động driver |
| `ASGN1_SET_MODE` | Chọn chế độ `[KERNEL]` prefix hoặc echo |
| `ASGN1_GET_VERSION` | Trả về chuỗi phiên bản driver |

Driver sử dụng `unlocked_ioctl`, mutex để đồng bộ dữ liệu và kiểm tra lệnh không hợp lệ.

### Task 2 — Misc Counter Driver

Driver `counter_driver` sử dụng `misc_register()` và cung cấp:

- `/dev/counter`: đọc giá trị counter (tự tăng sau mỗi lần đọc); ghi số nguyên để đặt lại giá trị.
- `atomic_t`: quản lý bộ đếm an toàn khi truy cập đồng thời.
- `/proc/counter_info`: hiển thị giá trị counter, số lần đọc và số lần ghi.
- Kiểm tra input không hợp lệ và xử lý file offset khi đọc.

### Source Code, Logs & Report

- [Task 1 — ioctl driver](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/task1_ioctl/)
- [Task 2 — misc counter driver](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/task2_misc/)
- [Test outputs & kernel logs](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/logs/)
- [Báo cáo PDF](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/bao_cao/SE205174_NguyenTranNamNguyen_Asgn01_BaoCao.pdf)
- [File nén nộp bài (.tar.gz)](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01.tar.gz)

## Development Environment

| Component | Environment |
|---|---|
| Host | Ubuntu Linux |
| Target | ARMv7-A Cortex-A9 |
| Emulation | QEMU `vexpress-a9` |
| Kernel | Linux 5.15 |
| Cross-compiler | `arm-linux-gnueabihf-` |
| Languages | C, Shell |

## Repository Structure

Cấu trúc dưới đây được tóm tắt từ các thư mục thực tế trong repository (các file ảnh, log và file build phụ được lược bớt để dễ đọc).

```text
LOS201/
├── LAB01/
│   ├── bao_cao/
│   │   └── SE205174_Lab01_BaoCao.pdf
│   ├── configs/
│   │   ├── busybox.config
│   │   ├── kernel.config
│   │   └── uboot.config
│   ├── output/
│   │   ├── initramfs.cpio.gz
│   │   ├── u-boot.bin
│   │   ├── vexpress-v2p-ca9.dtb
│   │   └── zImage
│   ├── rootfs/
│   │   └── initramfs/                 # Root filesystem, init, bin, dev, etc...
│   └── SE205174_NguyenTranNamNguyen_Lab01.tar.gz
│
├── LAB02/
│   ├── bao_cao/
│   │   ├── SE205174_Lab02_BaoCao.pdf
│   │   └── LAB02_*.png               # Ảnh kiểm thử driver, JFFS2, init...
│   ├── configs/
│   ├── driver/
│   │   ├── lab2_driver.c
│   │   ├── lab2_driver.ko
│   │   └── Makefile
│   ├── mtd/
│   │   ├── jffs2_data/
│   │   ├── rootfs.jffs2
│   │   ├── nand_dump.bin
│   │   ├── lab2_rootfs.squashfs
│   │   └── squashfs_root/
│   ├── output/
│   │   └── initramfs_lab2.cpio.gz
│   ├── rootfs/
│   │   └── initramfs/
│   └── SE205174_NguyenTranNamNguyen_Lab02.tar.gz
│
├── LAB03/
│   ├── apps/
│   │   ├── buggy_app.c
│   │   ├── buggy_app_fixed.c
│   │   ├── leaky_app.c
│   │   ├── leaky_app_fixed.c
│   │   ├── perf_target.c
│   │   └── perf_test_lab03.txt
│   ├── bao_cao/
│   │   ├── SE205174_Lab03_BaoCao.pdf
│   │   ├── latency_comparison.png
│   │   ├── latency_histogram.png
│   │   └── LAB03_*.png               # Ảnh GDB, Valgrind, sửa lỗi...
│   ├── logs/
│   │   ├── gdb_session.txt
│   │   ├── valgrind_report.txt
│   │   ├── strace_summary.txt
│   │   ├── perf_report.txt
│   │   ├── hist_standard.txt
│   │   └── hist_realtime.txt
│   └── output/
│       └── initramfs_lab3.cpio.gz
│
├── Assignment01/
│   ├── SE205174_NguyenTranNamNguyen_Asgn01.tar.gz
│   └── SE205174_NguyenTranNamNguyen_Asgn01/
│       ├── bao_cao/
│       │   └── SE205174_NguyenTranNamNguyen_Asgn01_BaoCao.pdf
│       ├── logs/
│       │   ├── dmesg_counter.txt
│       │   ├── dmesg_full.txt
│       │   ├── test_asgn1_output.txt
│       │   └── test_counter_output.txt
│       ├── task1_ioctl/
│       │   ├── asgn1_driver.c
│       │   ├── asgn1_driver.ko
│       │   ├── asgn1_ioctl.h
│       │   ├── Makefile
│       │   ├── test_asgn1.sh
│       │   └── test_ioctl.c
│       └── task2_misc/
│           ├── counter_driver.c
│           ├── counter_driver.ko
│           ├── Makefile
│           └── test_counter.sh
└── README.md
```

### Báo cáo PDF của từng bài

- [LAB01 — Báo cáo PDF](./LAB01/bao_cao/SE205174_Lab01_BaoCao.pdf)
- [LAB02 — Báo cáo PDF](./LAB02/bao_cao/SE205174_Lab02_BaoCao.pdf)
- [LAB03 — Báo cáo PDF](./LAB03/bao_cao/SE205174_Lab03_BaoCao.pdf)
- [Assignment 01 — Báo cáo PDF](./Assignment01/SE205174_NguyenTranNamNguyen_Asgn01/bao_cao/SE205174_NguyenTranNamNguyen_Asgn01_BaoCao.pdf)

> **Ghi chú:** README mô tả phạm vi và yêu cầu kỹ thuật của các bài theo tài liệu môn học. Kết quả thực hiện cụ thể, ảnh chụp, log và báo cáo cần được đối chiếu trong từng thư mục bài nộp.

**Repository:** https://github.com/nnguyen1606-dev/LOS201
