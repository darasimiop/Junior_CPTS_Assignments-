#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "kmlab_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ogunbinu-Peters");
MODULE_DESCRIPTION("PA3 Kernel Module");

typedef struct {
    struct list_head list;
    unsigned int pid;
    unsigned long cpu_time;
} proc_list;

static struct proc_dir_entry *proc_entry;
static struct timer_list my_timer;
static LIST_HEAD(proc_head);

void timer_callback(struct timer_list *timer) {
    struct proc_list *entry;
    unsigned long cpu_usage;
    
    list_for_each_entry(entry, &proc_head, list) {
        if (get_cpu_use(entry->pid, &cpu_usage) == 0) {
            entry->cpu_time = cpu_usage;
        }
    }
    
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
}

int kmlab_open(struct inode *inode, struct file *file) {
    return 0;
}

ssize_t kmlab_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos) {
    char buf[128];
    unsigned int pid;
    struct proc_list *new_proc;

    if (copy_from_user(buf, buffer, count)) return -EFAULT;
    sscanf(buf, "%u", &pid);

    new_proc = kmalloc(sizeof(proc_list), GFP_KERNEL);
    new_proc->pid = pid;
    new_proc->cpu_time = 0;
    list_add(&new_proc->list, &proc_head);
    return count;
}

ssize_t kmlab_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos) {
    struct proc_list *entry;
    char output[256];
    int len = 0;

    list_for_each_entry(entry, &proc_head, list) {
        len += snprintf(output + len, 256 - len, "PID: %u, CPU Time: %lu\n", entry->pid, entry->cpu_time);
    }

    if (*ppos >= len) return 0;
    if (copy_to_user(buffer, output, len)) return -EFAULT;
    *ppos += len;
    return len;
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .read = kmlab_read,
    .write = kmlab_write,
    .open = kmlab_open,
};

static int __init kmlab_init(void) {
    proc_entry = proc_create("status", 0666, NULL, &proc_fops);
    if (!proc_entry) return -ENOMEM;

    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
    return 0;
}

static void __exit kmlab_exit(void) {
    proc_remove(proc_entry);
    del_timer(&my_timer);
}

module_init(kmlab_init);
module_exit(kmlab_exit);
