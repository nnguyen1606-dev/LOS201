obj-m += sms_sensor.o
sms_sensor-objs := sms_sensor_driver.o

KDIR := $(HOME)/embedded_asgn2/kernel/linux-5.15
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KCFLAGS="-march=armv7-a" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean

