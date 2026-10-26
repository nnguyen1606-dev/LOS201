CC_HOST = gcc
CC_ARM = arm-linux-gnueabihf-gcc

CFLAGS = -D_GNU_SOURCE -std=gnu11 -O0 -g -Wall -Wextra -Werror -pthread
HOST_FLAGS = -DHOST_TEST

LIBS = -lrt -lm -pthread

SOURCES = sms_app.c \
          sensor_thread.c \
          processor_thread.c \
          alert_thread.c \
          logger_thread.c \
          circular_buffer.c

.PHONY: all arm host clean

all: arm host

arm:
	$(CC_ARM) $(CFLAGS) -static $(SOURCES) -o sms_app $(LIBS)

host:
	$(CC_HOST) $(CFLAGS) $(HOST_FLAGS) $(SOURCES) -o sms_app_x86 $(LIBS)

clean:
	rm -f sms_app sms_app_x86
