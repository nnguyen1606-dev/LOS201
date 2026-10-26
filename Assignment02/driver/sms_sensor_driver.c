#include "sms_sensor.h"
#include <linux/ioctl.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/random.h>
#include <linux/ktime.h>
#include <linux/math64.h>
#include <linux/delay.h>
static unsigned int sampling_interval_ms = 100;
static unsigned long read_count = 0;
static DEFINE_MUTEX(sensor_lock);
static struct proc_dir_entry *sms_proc_entry;
static unsigned int last_temp_x10 = 0;
static unsigned int last_humidity_x10 = 0;
static u64 driver_start_ms = 0;
static u64 last_sample_ms = 0;
/* Called when the module is loaded */
/* Called when an application opens the sensor device */
static int sms_sensor_open(struct inode *inode, struct file *file)
{
    /* Device opened: debug message disabled */
    return 0;
}

/* Called when an application closes the sensor device */
static int sms_sensor_release(struct inode *inode, struct file *file)
{
    /* Device closed: debug message disabled */
    return 0;
}

static ssize_t sms_sensor_read(struct file *file,
                               char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    char data[64];
    unsigned int temp;
    unsigned int humidity;
    unsigned int interval;
    u64 timestamp_ms;
    u64 now_ms;
    u64 wait_ms;
    int len;
    ssize_t ret;

    if (*ppos != 0)
        return 0;

    if (count == 0)
        return 0;

    /* Protect sampling time across concurrent readers */
    if (mutex_lock_interruptible(&sensor_lock))
        return -ERESTARTSYS;

    interval = sampling_interval_ms;
    now_ms = div_u64(ktime_get_ns(), 1000000);

    if (last_sample_ms != 0 &&
        now_ms - last_sample_ms < interval) {

        wait_ms = interval - (now_ms - last_sample_ms);

        mutex_unlock(&sensor_lock);

        msleep((unsigned int)wait_ms);

        if (mutex_lock_interruptible(&sensor_lock))
            return -ERESTARTSYS;
    }

    /*
     * Recheck after acquiring the mutex.
     * Another reader may have sampled while we slept.
     */
    now_ms = div_u64(ktime_get_ns(), 1000000);

    if (last_sample_ms != 0 &&
        now_ms - last_sample_ms < sampling_interval_ms) {
        mutex_unlock(&sensor_lock);
        return -EAGAIN;
    }

    temp = 150 + get_random_u32() % 301;
    humidity = 300 + get_random_u32() % 601;
timestamp_ms = div_u64(ktime_get_ns(), 1000000);
    len = scnprintf(data, sizeof(data),
                    "%llu,%u.%u,%u.%u\n",
                    (unsigned long long)timestamp_ms,
                    temp / 10, temp % 10,
                    humidity / 10, humidity % 10);

    ret = simple_read_from_buffer(buf, count, ppos,
                                  data, len);

    if (ret > 0) {
        last_sample_ms = now_ms;
        read_count++;
        last_temp_x10 = temp;
        last_humidity_x10 = humidity;
    }

    mutex_unlock(&sensor_lock);

    return ret;
}


static long sms_sensor_ioctl(struct file *file,
                             unsigned int cmd,
                             unsigned long arg)
{
    unsigned int new_interval;

    if (cmd != SMS_SET_INTERVAL)
        return -ENOTTY;

    if (copy_from_user(&new_interval,
                       (unsigned int __user *)arg,
                       sizeof(new_interval)))
        return -EFAULT;

    if (new_interval == 0)
        return -EINVAL;

    mutex_lock(&sensor_lock);
    sampling_interval_ms = new_interval;
    mutex_unlock(&sensor_lock);

    pr_info("sms_sensor: interval set to %u ms\n",
            new_interval);

    return 0;
}


static const struct file_operations sms_sensor_fops = {
    .owner = THIS_MODULE,
    .open = sms_sensor_open,
   .read = sms_sensor_read, 
.release = sms_sensor_release,
.unlocked_ioctl = sms_sensor_ioctl,
};

static struct miscdevice sms_sensor_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sms_sensor",
    .fops = &sms_sensor_fops,
};

static int sms_stats_show(struct seq_file *m, void *v)
{
    unsigned int interval;
    unsigned long total_samples;
    unsigned int temp;
    unsigned int humidity;
    u64 uptime_ms;

    mutex_lock(&sensor_lock);

    interval = sampling_interval_ms;
    total_samples = read_count;
    temp = last_temp_x10;
    humidity = last_humidity_x10;

    mutex_unlock(&sensor_lock);


uptime_ms = div_u64(ktime_get_ns(), 1000000) - driver_start_ms;
    seq_printf(m, "SMS Sensor Statistics\n");
    seq_printf(m, "Sampling interval: %u ms\n", interval);
seq_printf(m, "read_count: %lu\n", total_samples);
    seq_printf(m, "Last temperature: %u.%u C\n",
               temp / 10, temp % 10);

    seq_printf(m, "Last humidity: %u.%u %%\n",
               humidity / 10, humidity % 10);

seq_printf(m, "Driver uptime: %llu seconds\n",
           (unsigned long long)div_u64(uptime_ms, 1000));
    return 0;
}
static int sms_stats_open(struct inode *inode, struct file *file)
{
    return single_open(file, sms_stats_show, NULL);
}
static const struct proc_ops sms_stats_ops = {
    .proc_open = sms_stats_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};


static int __init sms_sensor_init(void)
{
    int ret;
driver_start_ms = div_u64(ktime_get_ns(), 1000000);
    ret = misc_register(&sms_sensor_device);

    if (ret) {
        pr_err("sms_sensor: failed to register device\n");
        return ret;
    }

   sms_proc_entry = proc_create("sms_stats", 0444, NULL,
                             &sms_stats_ops);

if (!sms_proc_entry) {
    pr_err("sms_sensor: failed to create /proc/sms_stats\n");
    misc_deregister(&sms_sensor_device);
    return -ENOMEM;
} 
pr_info("sms_sensor: module initialized\n");
    return 0;
}


/* Called when the module is removed */
static void __exit sms_sensor_exit(void)
{
    misc_deregister(&sms_sensor_device);
    pr_info("sms_sensor: module removed\n");
proc_remove(sms_proc_entry);

}
module_init(sms_sensor_init);
module_exit(sms_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Tran Nam Nguyen");
MODULE_DESCRIPTION("Sensor Monitoring System - Kernel Driver");
