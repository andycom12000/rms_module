#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include "mp2_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("12");
MODULE_DESCRIPTION("CS-423 MP2");

#define DEBUG 1
#define PID_LEN 10

// Linked list node definition
struct Node {
	char pid[PID_LEN];
	int pid_i;
	unsigned long exec_time;
	struct list_head list;
};

// Workqueue work structure
struct Work{
	struct work_struct my_work;
};

// Function prototypes
static ssize_t mp2_read(struct file *file, char __user *buffer, size_t count, loff_t *data);
static int mp2_write(struct file *file, const char *buffer, unsigned long count, loff_t *data);
void scan(const char *str, const char *format, ...);
void update_list(int x);

// API functions
void registerProc(void);
void unregisterProc(void);
void yieldProc(void);


// File Ops
static const struct file_operations mp2_fops = {
	.owner = THIS_MODULE,
	.read = mp2_read,
	.write = mp2_write,
};

// Global variable
static struct proc_dir_entry *mp2_file;
static struct proc_dir_entry *mp2_dir;
static char procfs_buffer[1024] = {'\0'};
static char temp_buffer[1024] = {'\0'};
static unsigned long procfs_buffer_size = 0;
struct Node mylist;
static DEFINE_SEMAPHORE(list_lock);


// mp2_init - Called when module is loaded
int __init mp2_init(void)
{
	//int i = 0;
	//struct Node *tmp = NULL;

	#ifdef DEBUG
  	printk(KERN_ALERT "MP2 MODULE LOADING\n");
	#endif

	// Create directory and file entry
	mp2_dir = proc_mkdir("mp2", NULL);
	mp2_file = proc_create("status", 0666, mp2_dir, &mp2_fops);
  	if(!mp2_file)
	{
		// Remove the directory before exiting
		remove_proc_entry("mp2", NULL);
		return -ENOMEM;
	}
   
	// Create linked list
	#ifdef DEBUG
	printk(KERN_ALERT "Create List");
	#endif
	INIT_LIST_HEAD(&mylist.list);
	
	#ifdef DEBUG
	printk(KERN_ALERT "MP2 MODULE LOADED\n");
	#endif

	return 0;   
}

// mp2_exit - Called when module is unloaded
void __exit mp2_exit(void)
{
	struct list_head *q, *pos;
	struct Node *tmp;

	#ifdef DEBUG
	printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
	#endif

	// Remove entries
	remove_proc_entry("status", mp2_dir);
	remove_proc_entry("mp2", NULL);

	// Deallocate linked list
	printk(KERN_ALERT "Freeing list...\n");
	list_for_each_safe(pos, q, &mylist.list)
	{
		tmp = list_entry(pos, struct Node, list);
		printk(KERN_ALERT "Freeing pid=%s", tmp->pid);
		list_del(pos);
		kfree(tmp);
	}

	printk(KERN_ALERT "Done\n");
	printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp2_init);
module_exit(mp2_exit);


static ssize_t mp2_read(struct file *file, char __user *buffer, size_t count, loff_t *data)
{
	int ret;
	struct Node *tmp = NULL;
	char retString[50] = {'\0'};
	unsigned long totalLength = 0;
	printk(KERN_ALERT "Reading...");
	
	// Reset buffer
	strcpy(temp_buffer, retString);
	
	// Traverse the linked list
	down(&list_lock);
	list_for_each_entry(tmp, &mylist.list, list)
	{
		snprintf(retString, 50, "pid=%s, time=%lu\n", tmp->pid, tmp->exec_time);
		strcat(temp_buffer, retString);
	
#ifdef DEBUG
		printk(KERN_ALERT "pid=%s, time=%lu\n", tmp->pid, tmp->exec_time);
#endif
	}
	up(&list_lock);

	// Pass result to user
	totalLength = strlen(temp_buffer);
	ret = copy_to_user(buffer, temp_buffer, totalLength);
	*data += totalLength - ret;

	if(*data > totalLength)
		return 0;
	else
		return totalLength;
}

static int mp2_write(struct file *file, const char *buffer, unsigned long count, loff_t *data)
{
	//struct Node *tmp = NULL;
	char operation = '\0';
	int pid = 0;

	printk(KERN_ALERT "Writing...");
	procfs_buffer_size = count;

	if(procfs_buffer_size > 1024)
		procfs_buffer_size = 1024;
	
	if(copy_from_user(procfs_buffer, buffer, count))
		return -EFAULT;
	
#ifdef DEBUG
	printk(KERN_ALERT "Get from user:%s, count=%lu\n", procfs_buffer, count);
#endif

	// Put the pid and current time into linked list
	//tmp = (struct Node*)kmalloc(sizeof(struct Node), 0);
	scan(procfs_buffer, "%d%c", &pid, &operation);
	//tmp->exec_time = 0;
	/*
	down(&list_lock);
	list_add(&(tmp->list), &(mylist.list));
	up(&list_lock);
	*/
#ifdef DEBUG
	printk(KERN_ALERT "pid=%d, op=%c\n", pid, operation);
#endif

	return procfs_buffer_size;
}

void scan(const char *str, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsscanf(str, format, args);
	va_end(args);
}

void update_list(int x)
{
	//int ret = 0;
	struct Node *tmp = NULL;

	down(&list_lock);
	list_for_each_entry(tmp, &mylist.list, list)
	{
		//ret = get_cpu_use(tmp->pid_i, &tmp->exec_time);
#ifdef DEBUG
		printk(KERN_ALERT "\tpid=%d, time=%lu\n", tmp->pid_i, tmp->exec_time);
#endif
	}
	up(&list_lock);
}

void registerProc(void)
{
#ifdef DEBUG
	printk(KERN_ALERT "Register called\n");
#endif
}

void unregisterProc(void)
{
#ifdef DEBUG
	printk(KERN_ALERT "Unregister called\n");
#endif
}

void yieldProc(void)
{
#ifdef DEBUG
	printk(KERN_ALERT "Yield called\n");
#endif
}
