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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Embedded Linux Lab");
MODULE_DESCRIPTION("LAB-02: Simple Character Device Driver");
MODULE_VERSION("1.0");

#define DEVICE_NAME  "lab2"
#define CLASS_NAME   "lab2_class"
#define BUFFER_SIZE  1024
#define DRIVER_MAJOR 240

static int major_number = DRIVER_MAJOR;
static char device_buffer[BUFFER_SIZE];
static int buffer_len = 0;
static int open_count = 0;

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
    if (len > BUFFER_SIZE - 1) {
        pr_warn("lab2_driver: write too large, truncating\n");
        len = BUFFER_SIZE - 1;
    }

    mutex_lock(&lab2_mutex);

    if (copy_from_user(device_buffer, buf, len)) {
        mutex_unlock(&lab2_mutex);
        return -EFAULT;
    }

    buffer_len = len;
    device_buffer[buffer_len] = '\0';

    mutex_unlock(&lab2_mutex);

    pr_info("lab2_driver: received %zu bytes\n", len);

    return len;
}

/* File operations */
static struct file_operations lab2_fops = {
    .owner   = THIS_MODULE,
    .open    = lab2_open,
    .release = lab2_release,
    .read    = lab2_read,
    .write   = lab2_write,
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

