#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "asgn1_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Tran Nam Nguyen - SE205174");
MODULE_DESCRIPTION("Assignment 01: Character Driver with ioctl");
MODULE_VERSION("1.0");

#define DEVICE_NAME  "asgn1"
#define CLASS_NAME   "asgn1_class"
#define BUFFER_SIZE  1024
#define DRIVER_MAJOR 241

static int major_number = DRIVER_MAJOR;
static char device_buffer[BUFFER_SIZE];
static int buffer_len = 0;
static int open_count = 0;
static int write_count = 0;
static int read_count = 0;
static int last_write_size = 0;
static int current_mode = 1;

static struct class *lab2_class = NULL;
static struct device *lab2_device = NULL;

static DEFINE_MUTEX(lab2_mutex);

/* Open device */
static int lab2_open(struct inode *inode, struct file *file)
{
    mutex_lock(&lab2_mutex);
    open_count++;
    mutex_unlock(&lab2_mutex);

    pr_info("lab2_driver: device opened (count=%d)\n",
            open_count);
    return 0;
}

/* Close device */
static int lab2_release(struct inode *inode, struct file *file)
{
    pr_info("lab2_driver: device closed\n");
    return 0;
}

/* Read data */
static ssize_t lab2_read(struct file *file,
                         char __user *buf,
                         size_t len,
                         loff_t *offset)
{
    int bytes_read = 0;

    mutex_lock(&lab2_mutex);

    if (*offset >= buffer_len) {
        mutex_unlock(&lab2_mutex);
        return 0;
    }

    if (len > buffer_len - *offset)
        len = buffer_len - *offset;

    if (copy_to_user(buf, device_buffer + *offset, len)) {
        mutex_unlock(&lab2_mutex);
        pr_err("lab2_driver: copy_to_user failed\n");
        return -EFAULT;
    }

    *offset += len;
    bytes_read = len;
	/* Count successful read operations */
if (bytes_read > 0){
    read_count++;
}
    mutex_unlock(&lab2_mutex);

    pr_info("lab2_driver: sent %d bytes to user\n",
            bytes_read);

    return bytes_read;
}

/* Write data */
static ssize_t lab2_write(struct file *file,
                          const char __user *buf,
                          size_t len,
                          loff_t *offset)
{
    const char prefix[] = "[KERNEL]";
    size_t prefix_len = sizeof(prefix) - 1;
    size_t data_len = len;

    mutex_lock(&lab2_mutex);

    if (current_mode == 0) {
        /* Kernel mode */

        if (data_len > BUFFER_SIZE - 1 - prefix_len)
            data_len = BUFFER_SIZE - 1 - prefix_len;

        if (copy_from_user(device_buffer + prefix_len,
                           buf, data_len)) {
            mutex_unlock(&lab2_mutex);
            return -EFAULT;
        }

        memcpy(device_buffer, prefix, prefix_len);

        buffer_len = prefix_len + data_len;

    } else {
        /* Echo mode */

        if (data_len > BUFFER_SIZE - 1)
            data_len = BUFFER_SIZE - 1;

        if (copy_from_user(device_buffer, buf, data_len)) {
            mutex_unlock(&lab2_mutex);
            return -EFAULT;
        }

        buffer_len = data_len;
    }

    device_buffer[buffer_len] = '\0';

    write_count++;
    last_write_size = data_len;

    mutex_unlock(&lab2_mutex);

    pr_info("asgn1_driver: received %zu bytes\n", data_len);

    return data_len;
}




/* Handle ioctl commands from userspace */
static long asgn1_ioctl(struct file *file,
                       unsigned int cmd,
                       unsigned long arg)
{
    struct asgn1_stats stats;
    int new_mode;
    const char version[64] = "asgn1_driver v1.0 - SE205174";

    switch (cmd) {

    case ASGN1_RESET_BUFFER:
        /* TODO: Clear buffer and reset counters */

mutex_lock(&lab2_mutex);

    device_buffer[0] = '\0';
    buffer_len = 0;
    write_count = 0;
    read_count = 0;
    last_write_size = 0;

    mutex_unlock(&lab2_mutex);

    pr_info("asgn1_driver: buffer and counters reset\n");

        break;

case ASGN1_GET_STATS:
    mutex_lock(&lab2_mutex);

    stats.open_count = open_count;
    stats.write_count = write_count;
    stats.read_count = read_count;
    stats.buffer_len = buffer_len;
    stats.last_write_size = last_write_size;

    mutex_unlock(&lab2_mutex);

    if (copy_to_user((struct asgn1_stats __user *)arg,
                     &stats,
                     sizeof(stats)))
        return -EFAULT;

    pr_info("asgn1_driver: GET_STATS called\n");

    break;	
    case ASGN1_SET_MODE:
        /* TODO: Change driver mode */

if (copy_from_user(&new_mode,
                       (int __user *)arg,
                       sizeof(new_mode)))
        return -EFAULT;

    if (new_mode != 0 && new_mode != 1)
        return -EINVAL;

    mutex_lock(&lab2_mutex);
    current_mode = new_mode;
    mutex_unlock(&lab2_mutex);

    pr_info("asgn1_driver: mode changed to %d\n", new_mode);
        break;

    case ASGN1_GET_VERSION:
        /* TODO: Return driver version */
if (copy_to_user((char __user *)arg,
                     version,
                     sizeof(version)))
        return -EFAULT;

    pr_info("asgn1_driver: GET_VERSION called\n");
        break;

    default:
        return -ENOTTY;
    }

    return 0;
}

/* File operations */
static struct file_operations lab2_fops = {
    .owner   = THIS_MODULE,
    .open    = lab2_open,
    .release = lab2_release,
    .read    = lab2_read,
    .write   = lab2_write,
	.unlocked_ioctl = asgn1_ioctl,
};

/* Module initialization */
/* ===== PART B: /proc interface ===== */

static struct proc_dir_entry *lab2_proc_entry;

static int lab2_proc_show(struct seq_file *m, void *v)
{
    mutex_lock(&lab2_mutex);

    seq_printf(m, "LAB02 Driver Information\n");
    seq_printf(m, "Major number: %d\n", major_number);
    seq_printf(m, "Open count: %d\n", open_count);
    seq_printf(m, "Buffer length: %d\n", buffer_len);

    mutex_unlock(&lab2_mutex);

    return 0;
}

static int lab2_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, lab2_proc_show, NULL);
}

static const struct proc_ops lab2_proc_ops = {
    .proc_open    = lab2_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};


/* ===== PART B: /sys interface ===== */

static ssize_t status_show(struct device *dev,
                           struct device_attribute *attr,
                           char *buf)
{
    int count;
    int length;

    mutex_lock(&lab2_mutex);

    count = open_count;
    length = buffer_len;

    mutex_unlock(&lab2_mutex);

    return scnprintf(buf, PAGE_SIZE,
                     "Driver: lab2\n"
                     "Major: %d\n"
                     "Open count: %d\n"
                     "Buffer length: %d\n",
                     major_number, count, length);
}

static DEVICE_ATTR_RO(status);
/* ===== Sysfs: buffer_len ===== */
/* Show the number of bytes currently stored in device_buffer */
static ssize_t buffer_len_show(struct device *dev,
                               struct device_attribute *attr,
                               char *buf)
{
    int length;

    mutex_lock(&lab2_mutex);
    length = buffer_len;
    mutex_unlock(&lab2_mutex);

    return scnprintf(buf, PAGE_SIZE, "%d\n", length);
}

static DEVICE_ATTR_RO(buffer_len);


/* ===== Sysfs: open_count ===== */
/* Show how many times /dev/lab2 has been opened */
static ssize_t open_count_show(struct device *dev,
                               struct device_attribute *attr,
                               char *buf)
{
    int count;

    mutex_lock(&lab2_mutex);
    count = open_count;
    mutex_unlock(&lab2_mutex);

    return scnprintf(buf, PAGE_SIZE, "%d\n", count);
}

static DEVICE_ATTR_RO(open_count);


/* ===== Sysfs: last_data ===== */
/* Show the latest data written to /dev/lab2 */
static ssize_t last_data_show(struct device *dev,
                              struct device_attribute *attr,
                              char *buf)
{
    ssize_t length;

    mutex_lock(&lab2_mutex);

    length = scnprintf(buf, PAGE_SIZE, "%.*s\n",
                       buffer_len, device_buffer);

    mutex_unlock(&lab2_mutex);

    return length;
}

static DEVICE_ATTR_RO(last_data);
static int __init lab2_init(void)
{
    int ret;

    pr_info("lab2_driver: initializing module\n");

    ret = register_chrdev(major_number,
                          DEVICE_NAME,
                          &lab2_fops);

    if (ret < 0) {
        pr_err("lab2_driver: register_chrdev failed: %d\n",
               ret);
        return ret;
    }

    lab2_class = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(lab2_class)) {
        ret = PTR_ERR(lab2_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return ret;
    }

    lab2_device = device_create(lab2_class,
                                NULL,
                                MKDEV(major_number, 0),
                                NULL,
                                DEVICE_NAME);

    if (IS_ERR(lab2_device)) {
        ret = PTR_ERR(lab2_device);
        class_destroy(lab2_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return ret;
    }
	/* Create /proc/lab2_info */
lab2_proc_entry = proc_create("lab2_info",
                              0444,
                              NULL,
                              &lab2_proc_ops);

if (!lab2_proc_entry) {
    pr_err("lab2_driver: failed to create /proc/lab2_info\n");

    device_destroy(lab2_class, MKDEV(major_number, 0));
    class_destroy(lab2_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    return -ENOMEM;
}

/* Create sysfs status attribute */
ret = device_create_file(lab2_device, &dev_attr_status);

if (ret) {
    pr_err("lab2_driver: failed to create sysfs status\n");

    proc_remove(lab2_proc_entry);
    device_destroy(lab2_class, MKDEV(major_number, 0));
    class_destroy(lab2_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    return ret;
}
/* Create additional sysfs attributes */
ret = device_create_file(lab2_device, &dev_attr_buffer_len);
if (ret)
    goto err_buffer_len;

ret = device_create_file(lab2_device, &dev_attr_open_count);
if (ret)
    goto err_open_count;

ret = device_create_file(lab2_device, &dev_attr_last_data);
if (ret)
    goto err_last_data;    
pr_info("lab2_driver: loaded, major=%d\n",
            major_number);

    return 0;
err_last_data:
    device_remove_file(lab2_device, &dev_attr_open_count);

err_open_count:
    device_remove_file(lab2_device, &dev_attr_buffer_len);

err_buffer_len:
    device_remove_file(lab2_device, &dev_attr_status);
    proc_remove(lab2_proc_entry);
    device_destroy(lab2_class, MKDEV(major_number, 0));
    class_destroy(lab2_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    return ret;
}

/* Module cleanup */
static void __exit lab2_exit(void)
{
    device_remove_file(lab2_device, &dev_attr_last_data);
device_remove_file(lab2_device, &dev_attr_open_count);
device_remove_file(lab2_device, &dev_attr_buffer_len);
device_remove_file(lab2_device, &dev_attr_status);

    proc_remove(lab2_proc_entry);

    device_destroy(lab2_class, MKDEV(major_number, 0));

    class_destroy(lab2_class);

    unregister_chrdev(major_number, DEVICE_NAME);

    pr_info("lab2_driver: module unloaded\n");
}

module_init(lab2_init);
module_exit(lab2_exit);

