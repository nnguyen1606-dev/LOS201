#!/bin/sh

echo "===== TASK 2: MISC COUNTER DRIVER TEST ====="

# Load driver if it is not already loaded
if ! grep -q '^counter_driver ' /proc/modules; then
    insmod /lib/modules/counter_driver.ko || exit 1
fi

# Create device node using the assigned minor number
MINOR=$(awk '$2 == "counter" {print $1}' /proc/misc)

if [ -z "$MINOR" ]; then
    echo "ERROR: counter device not registered"
    exit 1
fi

if [ ! -e /dev/counter ]; then
    mknod /dev/counter c 10 "$MINOR" || exit 1
fi

echo "===== INITIAL COUNTER INFO ====="
cat /proc/counter_info

echo "===== SET COUNTER TO 0 ====="
echo 0 > /dev/counter

echo "===== READ COUNTER TWICE ====="
cat /dev/counter
cat /dev/counter

echo "===== WRITE COUNTER = 100 ====="
echo 100 > /dev/counter

echo "===== READ AFTER WRITE ====="
cat /dev/counter

echo "===== TEST NEGATIVE NUMBER ====="
if echo -5 > /dev/counter; then
    echo "FAIL: negative value accepted"
else
    echo "PASS: negative value rejected"
fi

echo "===== TEST INVALID STRING ====="
if echo abc > /dev/counter; then
    echo "FAIL: invalid string accepted"
else
    echo "PASS: invalid string rejected"
fi

echo "===== FINAL COUNTER INFO ====="
cat /proc/counter_info

echo "===== TASK 2 TEST COMPLETED ====="
