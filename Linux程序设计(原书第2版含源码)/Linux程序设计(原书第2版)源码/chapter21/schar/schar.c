/*
 *	Example of simple character device driver (schar)
 */

#include <linux/module.h>

#if defined(CONFIG_SMP)
#define __SMP__
#endif

#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>	/* for timers */
#include <linux/fs.h>		/* file modes and device registration */
#include <linux/poll.h>		/* for poll */
#include <linux/wrapper.h>	/* mem_map_reserve,mem_map_unreserve */
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/init.h>

#include <asm/io.h>

#include "schar.h"

/* forward declarations for _fops */
static ssize_t schar_read(struct file *file, char *buf, size_t count, loff_t *offset);
static ssize_t schar_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static unsigned int schar_poll(struct file *file, poll_table *wait);
static int schar_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int schar_mmap(struct file *file, struct vm_area_struct *vma);
static int schar_open(struct inode *inode, struct file *file);
static int schar_release(struct inode *inode, struct file *file);

static struct file_operations schar_fops = {
	NULL,		/* llseek */
	schar_read,
	schar_write,
	NULL,		/* readdir */
	schar_poll,
	schar_ioctl,
	schar_mmap,
	schar_open,
	NULL,		/* flush */
	schar_release,
	NULL,		/* fsync */
	NULL,		/* fasync */
	NULL		/* lock */
};

/* data */
static long schar_pool, schar_data_read, schar_data_written;
static char *schar_buffer;

/* schar wait queues. kernel newer than 2.3.0 uses a different syntax
   for declaring wait queues so check for that. */
#if LINUX_VERSION_CODE > 0x20300
DECLARE_WAIT_QUEUE_HEAD(schar_wq);
DECLARE_WAIT_QUEUE_HEAD(schar_poll_read);
#else
static struct wait_queue *schar_wq = NULL;
static struct wait_queue *schar_poll_read = NULL;
#endif

/* timer */
static struct timer_list schar_timer;
static long timer_fires;

/* settable parameters */
static char *schar_name = NULL;
static char schar_debug = 1;
static unsigned schar_inc = SCHAR_INC;
static unsigned long schar_timer_delay = SCHAR_TIMER_DELAY;
static unsigned long schar_pool_min = SCHAR_POOL_MIN;

/* sysctl entries */
static char schar_proc_string[SCHAR_MAX_SYSCTL];
static struct ctl_table_header *schar_root_header = NULL;
static int schar_read_proc(ctl_table *ctl, int write, struct file *file,
			   void *buffer, size_t *lenp);

static ctl_table schar_sysctl_table[] = {
	{ DEV_SCHAR_ENTRY,	/* binary id */
	  "0",			/* name */
	  &schar_proc_string,	/* data */
	  SCHAR_MAX_SYSCTL,	/* max size of output */
	  0644,			/* mode */
	  0,			/* child - none */
	  &schar_read_proc },	/* set up text */
	{ 0 }
};

static ctl_table schar_dir[] = {
	{ DEV_SCHAR,		/* /proc/dev/schar */
	  "schar",		/* name */
	  NULL,
	  0,
	  0555,
	  schar_sysctl_table },	/* the child */
	{ 0 }
};

static ctl_table schar_root_dir[] = {
	{ CTL_DEV,		/* /proc/dev */
	  "dev",		/* name */
	  NULL,
	  0,
	  0555,
	  schar_dir },		/* the child */
	{ 0 }
};

/* module parameters and descriptions */
MODULE_PARM(schar_name, "s");
MODULE_PARM_DESC(schar_name, "Name of device");

MODULE_PARM(schar_debug, "1b");
MODULE_PARM_DESC(schar_debug, "Enable debugging messages");

MODULE_PARM(schar_inc, "1i");
MODULE_PARM_DESC(schar_inc, "Byte increase in pool per timer fire");

MODULE_PARM(schar_timer_delay, "1l");
MODULE_PARM_DESC(schar_timer_delay, "Timer ticks between timer fire");

MODULE_PARM(schar_pool_min, "1l");
MODULE_PARM_DESC(schar_pool_min, "Timer fill pool minimum in bytes");

MODULE_DESCRIPTION("schar, Sample character driver");
MODULE_AUTHOR("Jens Axboe");

static void schar_timer_handler(unsigned long data)
{
	timer_fires++;

	/* setup timer again */
	if (schar_pool < schar_pool_min) {
		schar_pool += SCHAR_INC;
		schar_timer.expires = jiffies + schar_timer_delay;
		schar_timer.function = &schar_timer_handler;
		add_timer(&schar_timer);
		MSG("setting timer up again, %ld data now\n", schar_pool);
	}

	/* if the module is in use, we most likely has a reader waiting for
	   data. wake up any processes that are waiting for data. */
	if (MOD_IN_USE && schar_pool > 0) {
		wake_up_interruptible(&schar_wq);
		wake_up_interruptible(&schar_poll_read);
	}
	
	return;
}

static int schar_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long size;

	/* mmap flags - could be read and write, also */
	MSG("mmap: %s\n", vma->vm_flags & VM_WRITE ? "write" : "read");
	
	/* we will not accept an offset into the page */
	if(vma->vm_offset != 0) {
		MSG("mmap: offset must be 0\n");
		return -EINVAL;
	}
		
	/* schar_buffer is only one page */
	size = vma->vm_end - vma->vm_start;
	if (size != PAGE_SIZE) {
		MSG("mmap: wanted %lu, but PAGE_SIZE is %lu\n", size, PAGE_SIZE);
		return -EINVAL;
	}
	
	/* remap user buffer */
	if (remap_page_range(vma->vm_start, virt_to_phys(schar_buffer), size,
			     vma->vm_page_prot))
		return -EAGAIN;
		
	return 0;
}

static int schar_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{

	/* make sure that the command is really one of schar's */
	if (_IOC_TYPE(cmd) != SCHAR_IOCTL_BASE)
		return -ENOTTY;
		
	switch (cmd) {
		case SCHAR_TOGGLE_DEBUG: {
			schar_debug = !schar_debug;
			return 0;
		}

		case SCHAR_GET_POOL: {
			if (put_user(schar_pool, (unsigned long *)arg))
				return -EFAULT;
			break;
		}

		case SCHAR_GET_TIMER_FIRES: {
			if (put_user(timer_fires, (unsigned long *)arg))
				return -EFAULT;
			break;
		}
				
		case SCHAR_GET_READ: {
			if (put_user(schar_data_read, (unsigned long *)arg))
				return -EFAULT;
			break;
		}

		case SCHAR_GET_WRITTEN: {
			if (put_user(schar_data_read, (unsigned long *)arg))
				return -EFAULT;
			break;
		}

		case SCHAR_EX_TIMER_DELAY: {
			unsigned long tmp = schar_timer_delay;
			if (!capable(CAP_SYS_ADMIN))
				return -EACCES;
			if (get_user(schar_timer_delay, (unsigned long *)arg))
				return -EFAULT;
			if (put_user(tmp, (unsigned long *)arg))
				return -EFAULT;
			break;
		}

		case SCHAR_EX_POOL_MIN: {
			unsigned long tmp = schar_pool_min;
			if (get_user(schar_pool_min, (unsigned long *)arg))
				return -EFAULT;
			if (put_user(tmp, (unsigned long *)arg))
				return -EFAULT;
			break;
		}

		case SCHAR_EX_TIMER_INC: {
			unsigned tmp = schar_inc;
			if (get_user(schar_inc, (unsigned *)arg))
				return -EFAULT;
			if (put_user(tmp, (unsigned*)arg))
				return -EFAULT;
			break;
		}
		
		default: {
			MSG("ioctl: no such command\n");
			return -ENOTTY;
		}
	}

	/* to keep gcc happy */
	return 0;
}

static int schar_read_proc(ctl_table *ctl, int write, struct file *file,
			   void *buffer, size_t *lenp)
{
	int len = 0;
	
	MSG("proc: %s\n", write ? "write" : "read");

	/* someone is writing data to us */
	if (write) {
		char *tmp = (char *) get_free_page(GFP_KERNEL);
		MSG("proc: someone wrote %u bytes\n", (unsigned)*lenp);
		if (tmp) {
			if (!copy_from_user(tmp, buffer, *lenp))
				MSG("proc: %s", tmp);
			free_page((unsigned long)tmp);
			file->f_pos += *lenp;
		}
		return 0;
	}
	
	len += sprintf(schar_proc_string, "schar\n\n");
	len += sprintf(schar_proc_string+len, "schar_pool\t\t\t%ld\n",
						schar_pool);
	len += sprintf(schar_proc_string+len, "data read\t\t\t%ld\n",
						schar_data_read);
	len += sprintf(schar_proc_string+len, "data written\t\t\t%ld\n",
						schar_data_written);
	len += sprintf(schar_proc_string+len, "timer fired\t\t\t%ld times\n",
						timer_fires);
	len += sprintf(schar_proc_string+len, "\nchangable parameters:\n\n");
	len += sprintf(schar_proc_string+len, "debugging %sabled\n",
						schar_debug ? "en" : "dis");
	len += sprintf(schar_proc_string+len, "bytes increase for timer\t%u\n",
						schar_inc);
	len += sprintf(schar_proc_string+len, "timer delay\t\t\t%lu\n",
						schar_timer_delay);
	len += sprintf(schar_proc_string+len, "pool byte minimum\t\t%lu\n",
						schar_pool_min);
	
	*lenp = len;
	return proc_dostring(ctl, write, file, buffer, lenp);
	
	
}

/* increment and decrement usage count when someone is accessing
   /proc/sys/dev/schar */
static void schar_fill_inode(struct inode *inode, int fill)
{
	if (fill) {
		MOD_INC_USE_COUNT;
	} else {
		MOD_DEC_USE_COUNT;
	}
}

static ssize_t schar_read(struct file *file, char *buf, size_t count,
			  loff_t *offset)
{

	/* if less data than requested is here, put reading process to sleep */
	while (count > schar_pool) {
		/* if the device is opened non blocking satisfy what we
		   can of the request and don't put the process to sleep. */
		if (file->f_flags & O_NONBLOCK) {
			if (schar_pool > 0) {
				file->f_pos += schar_pool;
				schar_data_read += file->f_pos;
				if(copy_to_user(buf, schar_buffer, schar_pool))
					return -EFAULT;
				count = schar_pool;
				schar_pool = 0;
				return count;
			} else {
				return -EAGAIN;
			}
		}
			
		MSG("putting process with pid %u to sleep\n", current->pid);
		/* go to sleep, but wake up on signals */
		interruptible_sleep_on(&schar_wq);
		if (signal_pending(current)) {
			MSG("pid %u got signal\n", (unsigned)current->pid);
			/* tell vfs about the signal */
			return -EINTR;
		}
	}

	/* copy the data the our buffer */
	if (copy_to_user(buf, schar_buffer, count))
		return -EFAULT;

	schar_pool -= count;
	schar_data_read += count;

	MSG("want to read %u bytes, %ld bytes in queue\n", (unsigned)count,
							    schar_pool);
	
	/* return data written */
	file->f_pos += count;
	return count;
}

static ssize_t schar_write(struct file *file, const char *buf, size_t count,
			   loff_t *offset)
{
	schar_pool += count;
	schar_data_written += count;

	/* if we were really writing - modify the file position to
	    reflect the amount of data written */
	file->f_pos += count;
	if (copy_from_user(schar_buffer, buf, count))
		return -EFAULT;
	
	/* wake up reading processes, if any */
	if (schar_pool > 0) {
		wake_up_interruptible(&schar_wq);
		wake_up_interruptible(&schar_poll_read);
	}
	
	MSG("trying to write %u bytes, %ld bytes in queue now\n",
					(unsigned)count, schar_pool);

	/* return data written */
	return count;
}

static unsigned int schar_poll(struct file *file, poll_table * wait)
{
	unsigned int mask = 0;

	poll_wait(file, &schar_poll_read, wait);
	
	/* if the pool contains data, a read will succeed */
	if (schar_pool > 0)
		mask |= POLLIN | POLLRDNORM;
		
	/* a write always succeeds */
	mask |= POLLOUT | POLLWRNORM;
	
	return mask;
}

static int schar_open(struct inode *inode, struct file *file)
{
	/* increment usage count */
	MOD_INC_USE_COUNT;
	
	if (file->f_mode & FMODE_READ) {
		MSG("opened for reading\n");
		/* we have a reader, set up the timer if it isn't set */
		if (!timer_pending(&schar_timer)) {
			schar_timer.function = &schar_timer_handler;
			mod_timer(&schar_timer, SCHAR_TIMER_DELAY);
		}
	} else if (file->f_mode & FMODE_WRITE) {
		MSG("opened for writing \n");
	}

	MSG("major: %d minor: %d\n", MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	return 0;
}
static int schar_release(struct inode *inode, struct file *file)
{
	MOD_DEC_USE_COUNT;
	MSG("schar_release\n");
	return 0;
}


int init_module(void)
{
	int res;
	
	if (schar_name == NULL)
		schar_name = "schar";
		
	/* get a free page */
	schar_buffer = (char *) __get_free_page(GFP_KERNEL);
	if (schar_buffer == NULL)
		return -ENOMEM;

	/* reserve the page and fill it with garbage */
	mem_map_reserve(MAP_NR(schar_buffer));
	memset(schar_buffer, 0x59, PAGE_SIZE);
	
	/* register device with kernel */
	res = register_chrdev(SCHAR_MAJOR, schar_name, &schar_fops);
	if (res) {
		MSG("can't register device with kernel\n");
		return res;
	}
	
	/* register proc entry */
	schar_root_header = register_sysctl_table(schar_root_dir, 0);
	schar_root_dir->child->de->fill_inode = &schar_fill_inode;
	
	/* init statistics vars */
	timer_fires = schar_pool = schar_data_read = schar_data_written = 0;
	
	/* initialize timer */
	init_timer(&schar_timer);
	
	/* all done */	
	MSG("module loaded\n");
	return 0;
}

void cleanup_module(void)
{
	/* unregister device and proc entry */
	unregister_chrdev(SCHAR_MAJOR, "schar");
	if (schar_root_header)
		unregister_sysctl_table(schar_root_header);
	
	/* unreserve page and free it */
	mem_map_unreserve(MAP_NR(schar_buffer));
	free_page((unsigned long) schar_buffer);

	if (timer_pending(&schar_timer))
		del_timer(&schar_timer);
	
	MSG("unloaded\n");
	return;
}

