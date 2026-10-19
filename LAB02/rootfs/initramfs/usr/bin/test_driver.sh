#!/bin/sh

echo "================================"
echo " LAB02 - DRIVER TEST"
echo "================================"

echo "[TEST 1] Check loaded module"
lsmod | grep lab2_driver

echo "[TEST 2] Check device node"
ls -l /dev/lab2

echo "[TEST 3] Write data to device"
echo "Hello from test_driver.sh" > /dev/lab2

echo "[TEST 4] Read data from device"
cat /dev/lab2

echo "[TEST 5] Check driver information"
cat /proc/lab2_info

echo "[TEST 6] Check kernel log"
dmesg | grep lab2

echo "================================"
echo " DRIVER TEST COMPLETED"
echo "================================"
