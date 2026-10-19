#!/bin/sh

echo "================================"
echo " LAB02 - PROCFS AND SYSFS TEST"
echo "================================"

echo "[TEST 1] Read /proc/lab2_info"
cat /proc/lab2_info

echo "[TEST 2] List sysfs attributes"
ls /sys/class/lab2_class/lab2/

echo "[TEST 3] Read buffer_len"
cat /sys/class/lab2_class/lab2/buffer_len

echo "[TEST 4] Read open_count"
cat /sys/class/lab2_class/lab2/open_count

echo "[TEST 5] Read last_data"
cat /sys/class/lab2_class/lab2/last_data

echo "[TEST 6] Write new data"
echo "LAB02 Procfs Sysfs Test" > /dev/lab2

echo "[TEST 7] Read updated /proc information"
cat /proc/lab2_info

echo "[TEST 8] Read updated sysfs attributes"

echo "buffer_len:"
cat /sys/class/lab2_class/lab2/buffer_len

echo "open_count:"
cat /sys/class/lab2_class/lab2/open_count

echo "last_data:"
cat /sys/class/lab2_class/lab2/last_data

echo "================================"
echo " PROCFS SYSFS TEST COMPLETED"
echo "================================"
