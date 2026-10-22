#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "asgn1_ioctl.h"

int main(void)
{
    int fd;
    char version[64] = {0};
    struct asgn1_stats stats = {0};

    fd = open("/dev/asgn1", O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* Test GET_VERSION */
    if (ioctl(fd, ASGN1_GET_VERSION, version) < 0) {
        perror("GET_VERSION");
    } else {
        printf("Driver version: %s\n", version);
    }

    /* Test GET_STATS */
    if (ioctl(fd, ASGN1_GET_STATS, &stats) < 0) {
        perror("GET_STATS");
    } else {
        printf("Open count: %d\n", stats.open_count);
        printf("Write count: %d\n", stats.write_count);
        printf("Read count: %d\n", stats.read_count);
        printf("Buffer length: %d\n", stats.buffer_len);
        printf("Last write size: %d\n", stats.last_write_size);
    }

    /* Test SET_MODE */
    int mode = 0;

    if (ioctl(fd, ASGN1_SET_MODE, &mode) < 0) {
        perror("SET_MODE");
    } else {
        printf("SET_MODE: Kernel mode enabled\n");
    }

    /* Write data in Kernel mode */
    const char *message = "Hello";

    if (write(fd, message, 5) < 0) {
        perror("write");
    }

    /* Các đoạn kiểm thử READ và RESET_BUFFER
       sẽ được thêm tiếp ở vị trí này */

/* Read data in Kernel mode */
char buffer[128] = {0};
ssize_t n;

lseek(fd, 0, SEEK_SET);



    
    n = read(fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
    } else {
        buffer[n] = '\0';
        printf("Kernel mode result: %s\n", buffer);
    }
    /* Test RESET_BUFFER */
    if (ioctl(fd, ASGN1_RESET_BUFFER) < 0) {
        perror("RESET_BUFFER");
    } else {
        printf("RESET_BUFFER: Success\n");
    }

    /* Check statistics after reset */
    if (ioctl(fd, ASGN1_GET_STATS, &stats) < 0) {
        perror("GET_STATS after reset");
    } else {
        printf("Write count after reset: %d\n", stats.write_count);
        printf("Read count after reset: %d\n", stats.read_count);
        printf("Buffer length after reset: %d\n", stats.buffer_len);
    }

    /* Test Echo mode */
    mode = 1;

    if (ioctl(fd, ASGN1_SET_MODE, &mode) < 0) {
        perror("SET_MODE Echo");
    } else {
        printf("SET_MODE: Echo mode enabled\n");
    }

/* Write data in Echo mode */
if (write(fd, "Hello Echo", 10) < 0) {
    perror("write Echo");
}

/* Reopen device to reset read offset */
close(fd);

fd = open("/dev/asgn1", O_RDWR);

if (fd < 0) {
    perror("open Echo");
    return 1;
}

    /* Read data in Echo mode */
   

    n = read(fd, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read Echo");
    } else {
        buffer[n] = '\0';
        printf("Echo mode result: %s\n", buffer);
    }

    /* Read data in Echo mode */
    n = read(fd, buffer, sizeof(buffer) - 1);

    if (n < 0) {
        perror("read Echo");
    } else {
        buffer[n] = '\0';
        printf("Echo mode result: %s\n", buffer);
    }

    /* Test invalid mode */
    mode = 5;

    if (ioctl(fd, ASGN1_SET_MODE, &mode) < 0) {
        perror("SET_MODE invalid");
        printf("Invalid mode rejected successfully\n");
    } else {
        printf("ERROR: Invalid mode was accepted\n");
    }

    /* Test unsupported ioctl command */
    if (ioctl(fd, _IO('A', 99)) < 0) {
        perror("Unsupported ioctl");
        printf("Unsupported ioctl rejected successfully\n");
    } else {
        printf("ERROR: Unsupported ioctl was accepted\n");
    }

    close(fd);
    return 0;
}


  
