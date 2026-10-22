#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Tran Nam Nguyen - SE205174");
MODULE_DESCRIPTION("Assignment 01 - Task 2: Misc Counter Driver");
MODULE_VERSION("1.0");
#define DEVICE_NAME "counter"

static atomic_t counter = ATOMIC_INIT(0);

static atomic_t read_count = ATOMIC_INIT(0);
static atomic_t write_count = ATOMIC_INIT(0);

static struct proc_dir_entry *counter_proc_entry;
/* Read current counter value and increment it */
static ssize_t counter_read(struct file *file,
                            char __user *buf,
                            size_t count,
                            loff_t *ppos)
{
    char temp[32];
    int value;
    int len;

    /* Return EOF if this read has already been completed */
    if (*ppos > 0)
        return 0;

    /* Read current value and increment counter atomically */
    value = atomic_inc_return(&counter) - 1;

    /* Convert integer to string */
    len = scnprintf(temp, sizeof(temp), "%d\n", value);

    /* Copy data from kernel to userspace */
    if (count < len)
        return -EINVAL;

    if (copy_to_user(buf, temp, len))
        return -EFAULT;

    atomic_inc(&read_count);

    *ppos += len;

    pr_info("counter_driver: read value=%d\n", value);

    return len;
}
/* Set counter value from userspace */
static ssize_t counter_write(struct file *file,
                             const char __user *buf,
                             size_t count,
                             loff_t *ppos)
{
    char temp[32];
    int value;

    /* Validate input length */
    if (count == 0 || count >= sizeof(temp))
        return -EINVAL;

    /* Copy data from userspace */
    if (copy_from_user(temp, buf, count))
        return -EFAULT;

    temp[count] = '\0';

    /* Convert string to integer */
    if (kstrtoint(temp, 10, &value))
        return -EINVAL;

    /* Reject negative values */
    if (value < 0)
        return -EINVAL;

    /* Update counter atomically */
    atomic_set(&counter, value);

    atomic_inc(&write_count);

    pr_info("counter_driver: counter set to %d\n", value);

    return count;
}
/* Display counter statistics through procfs */
static int counter_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Counter Driver Information\n");
    seq_printf(m, "Current counter: %d\n",
               atomic_read(&counter));
    seq_printf(m, "Read count: %d\n",
               atomic_read(&read_count));
    seq_printf(m, "Write count: %d\n",
               atomic_read(&write_count));

    return 0;
}

/* Open /proc/counter_info */
static int counter_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, counter_proc_show, NULL);
}
/* File operations for /dev/counter */
static const struct file_operations counter_fops = {
    .owner = THIS_MODULE,
    .read = counter_read,
    .write = counter_write,
    .llseek = no_llseek,
};

/* Register counter as a miscellaneous device */
static struct miscdevice counter_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &counter_fops,
    .mode = 0666,
};

/* File operations for /proc/counter_info */
static const struct proc_ops counter_proc_ops = {
    .proc_open = counter_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};
/* Initialize counter driver */
static int __init counter_init(void)
{
    int ret;

    /* Register miscellaneous device */
    ret = misc_register(&counter_miscdev);

    if (ret) {
        pr_err("counter_driver: misc_register failed\n");
        return ret;
    }

    /* Create /proc/counter_info */
    counter_proc_entry = proc_create(
        "counter_info",
        0444,
        NULL,
        &counter_proc_ops
    );

    if (!counter_proc_entry) {
        pr_err("counter_driver: proc_create failed\n");

        misc_deregister(&counter_miscdev);

        return -ENOMEM;
    }

    pr_info("counter_driver: module loaded successfully\n");

    return 0;
}

/* Cleanup counter driver */
static void __exit counter_exit(void)
{
    /* Remove procfs entry */
    proc_remove(counter_proc_entry);

    /* Unregister miscellaneous device */
    misc_deregister(&counter_miscdev);

    pr_info("counter_driver: module unloaded\n");
}

module_init(counter_init);
module_exit(counter_exit);
