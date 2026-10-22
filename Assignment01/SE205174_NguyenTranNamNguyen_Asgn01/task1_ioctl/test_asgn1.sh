#!/bin/sh

echo "===== ASSIGNMENT 01 - TASK 1 TEST ====="

echo "[1] Loading driver..."
insmod /lib/modules/asgn1_driver.ko || exit 1

echo "[2] Creating device node..."
if [ ! -e /dev/asgn1 ]; then
    mknod /dev/asgn1 c 241 0 || exit 1
fi

echo "[3] Checking device..."
ls -l /dev/asgn1

echo "[4] Running ioctl tests..."
/bin/test_ioctl

echo "[5] Checking kernel messages..."
dmesg | grep -E 'asgn1_driver|lab2_driver'

echo "[6] Unloading driver..."
rmmod asgn1_driver

echo "===== TASK 1 TEST COMPLETED ====="

