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

```text
LOS201/
├── LAB01/
├── LAB02/
├── LAB03/
├── Assignment01/
│   ├── SE205174_NguyenTranNamNguyen_Asgn01.tar.gz
│   └── SE205174_NguyenTranNamNguyen_Asgn01/
│       ├── task1_ioctl/
│       ├── task2_misc/
│       ├── logs/
│       └── bao_cao/
└── README.md
```

> **Ghi chú:** README mô tả phạm vi và yêu cầu kỹ thuật của các bài theo tài liệu môn học. Kết quả thực hiện cụ thể, ảnh chụp, log và báo cáo cần được đối chiếu trong từng thư mục bài nộp.

**Repository:** https://github.com/nnguyen1606-dev/LOS201
