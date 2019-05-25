/*
 * Sample Ram DIsk Module, Radimo
 *
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
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>

#include "radimo.h"

#define MAJOR_NR		RADIMO_MAJOR
#define DEVICE_NAME		"radimo"
#define DEVICE_REQUEST		radimo_request
#define DEVICE_NR(device)	(MINOR(device))
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define DEVICE_NO_RANDOM

#include <linux/blk.h>

#define RADIMO_HARDS_BITS	9	/* 2**9 byte hardware sector */
#define RADIMO_BLOCK_SIZE	1024	/* block size */
#define RADIMO_TOTAL_SIZE	2048	/* size in blocks */

/* the storage pool */
#ifndef RADIMO_CHEAT_REQUEST
static char *radimo_storage;
#endif

static int radimo_hard = 1 << RADIMO_HARDS_BITS;
static int radimo_soft = RADIMO_BLOCK_SIZE;
static int radimo_size = RADIMO_TOTAL_SIZE;

static int radimo_readahead = 4;

/* for media changes */
static int radimo_changed;
struct timer_list radimo_timer;

/* module parameters and descriptions */
MODULE_PARM(radimo_soft, "1i");
MODULE_PARM_DESC(radimo_soft, "Software block size");
MODULE_PARM(radimo_size, "1i");
MODULE_PARM_DESC(radimo_size, "Total size in KB");
MODULE_PARM(radimo_readahead, "1i");
MODULE_PARM_DESC(radimo_readahead, "Max number of sectors to read ahead");

/* forward declarations for _fops */
static int radimo_open(struct inode *inode, struct file *file);
static int radimo_release(struct inode *inode, struct file *file);
static int radimo_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);
static int radimo_media_change(kdev_t dev);
static int radimo_revalidate(kdev_t dev);

static struct file_operations radimo_fops = {
	NULL,			/* llseek */
	block_read,
	block_write,
	NULL,			/* readdir */
	NULL,			/* poll */
	radimo_ioctl,
	NULL,			/* mmap */
	radimo_open,
	NULL,			/* flush */
	radimo_release,
	NULL,			/* fsync */ 
	NULL,			/* fasync */
	radimo_media_change,
	radimo_revalidate,	/* revalidate */
	NULL			/* lock */
};

void radimo_request(void)
{
	unsigned long offset, total;
	
radimo_begin:

	INIT_REQUEST;

	MSG(RADIMO_REQUEST, "%s sector %lu of %lu\n",
				CURRENT->cmd == READ ? "read" : "write",
				CURRENT->sector,
				CURRENT->current_nr_sectors);

	offset = CURRENT->sector * radimo_hard;
	total = CURRENT->current_nr_sectors * radimo_hard;
	
	/* access beyond end of the device */
	if (total+offset > radimo_size * (radimo_hard << 1)) {
		/* error in request  */
		end_request(0);
		goto radimo_begin;
	}

	MSG(RADIMO_REQUEST, "offset = %lu, total = %lu\n", offset, total);
	
#ifdef RADIMO_CHEAT_REQUEST
	/* if we actually get the read, that particular block hasn't
	   been written yet - just serve zeroes. for writing we lock
	   the buffer */
	if (CURRENT->cmd == READ) {
		memset(CURRENT->buffer, 0, total);
	} else if (CURRENT->cmd == WRITE) {
		set_bit(BH_Protected, &CURRENT->bh->b_state);
	} else {
		/* can't happen */
		MSG(RADIMO_ERROR, "cmd == %d is invalid\n", CURRENT->cmd);
		end_request(0);
	}
#else
	if (CURRENT->cmd == READ) {
		memcpy(CURRENT->buffer, radimo_storage+offset, total);
	} else if (CURRENT->cmd == WRITE) {
		memcpy(radimo_storage+offset, CURRENT->buffer, total);
	} else {
		/* can't happen */
		MSG(RADIMO_ERROR, "cmd == %d is invalid\n", CURRENT->cmd);
		end_request(0);
	}
#endif

	/* successful */
	end_request(1);

	/* let INIT_REQUEST return when we are done */
	goto radimo_begin;
}

static int radimo_media_change(kdev_t dev)
{
	if (radimo_changed)
		MSG(RADIMO_INFO, "media has changed\n");
	
	/* 0 means medium has not changed, while 1 indicates a change */
	return radimo_changed;
}

static int radimo_revalidate(kdev_t dev)
{
	MSG(RADIMO_INFO, "revalidate\n");
	
	/* just return 0, check_disk_change ignores it anyway */
	return 0;
}

void radimo_timer_fn(unsigned long data)
{
	MSG(RADIMO_TIMER, "timer expired\n");

	/* only "change media" if device is unused */
	if (MOD_IN_USE) {
		radimo_changed = 0;
	} else {
		/* medium changed, clear storage and */
		radimo_changed = 1;
#ifndef RADIMO_CHEAT_REQUEST
		memset(radimo_storage, 0, 1024*radimo_size);
#endif
		/* data contains i_rdev */
		fsync_dev(data);
		invalidate_buffers(data);
	}

	/* set it up again */
	radimo_timer.expires = RADIMO_TIMER_DELAY + jiffies;
	add_timer(&radimo_timer);
}

static int radimo_release(struct inode *inode, struct file *file)
{
	MSG(RADIMO_OPEN, "closed\n");
	MOD_DEC_USE_COUNT;
	return 0;
}

static int radimo_open(struct inode *inode, struct file *file)
{
	MSG(RADIMO_OPEN, "opened\n");
	MOD_INC_USE_COUNT;

	/* timer function needs device to invalidate buffers. pass it as
	   data. */
	radimo_timer.data = inode->i_rdev;
	radimo_timer.expires = RADIMO_TIMER_DELAY + jiffies;
	radimo_timer.function = &radimo_timer_fn;

	if (!timer_pending(&radimo_timer))
		add_timer(&radimo_timer);

	return 0;
}

static int radimo_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	unsigned int minor;
	
	if (!inode || !inode->i_rdev) 	
		return -EINVAL;

	minor = MINOR(inode->i_rdev);

	switch (cmd) {

		case BLKFLSBUF: {
			/* flush buffers */
			MSG(RADIMO_IOCTL, "ioctl: BLKFLSBUF\n");
			/* deny all but root */
			if (!capable(CAP_SYS_ADMIN))
				return -EACCES;
			fsync_dev(inode->i_rdev);
			invalidate_buffers(inode->i_rdev);
			break;
		}

         	case BLKGETSIZE: {
			/* return device size */
			MSG(RADIMO_IOCTL, "ioctl: BLKGETSIZE\n");
			if (!arg)
				return -EINVAL;
			return put_user(radimo_size*2, (long *) arg);
		}
		
		case BLKRASET: {
			/* set read ahead value */
			int tmp;
			MSG(RADIMO_IOCTL, "ioctl: BLKRASET\n");
			if (get_user(tmp, (long *)arg))
				return -EINVAL;
			if (tmp > 0xff)
				return -EINVAL;
			read_ahead[RADIMO_MAJOR] = tmp;
			return 0;
		}

		case BLKRAGET: {
			/* return read ahead value */
			MSG(RADIMO_IOCTL, "ioctl: BLKRAGET\n");
			if (!arg)
				return -EINVAL;
			return put_user(read_ahead[RADIMO_MAJOR], (long *)arg);
		}

		case BLKSSZGET: {
			/* return block size */
			MSG(RADIMO_IOCTL, "ioctl: BLKSSZGET\n");
			if (!arg)
				return -EINVAL;
			return put_user(radimo_soft, (long *)arg);
		}

		default: {
			MSG(RADIMO_ERROR, "ioctl wanted %u\n", cmd);
			return -ENOTTY;
		}
	}

	return 0;
}
	
int init_module(void)
{
	int res;
	
	/* block size must be a multiple of sector size */
	if (radimo_soft & ((1 << RADIMO_HARDS_BITS)-1)) {
		MSG(RADIMO_ERROR, "Block size not a multiple of sector size\n");
		return -EINVAL;
	}
	
#ifndef RADIMO_CHEAT_REQUEST
	/* allocate room for data */
	radimo_storage = (char *) vmalloc(1024*radimo_size);
	if (radimo_storage == NULL) {
		MSG(RADIMO_ERROR, "Not enough memory. Try a smaller size.\n");
		return -ENOMEM;
	}
	memset(radimo_storage, 0, 1024*radimo_size);
#endif
	
	/* register block device */
	res = register_blkdev(RADIMO_MAJOR, "radimo", &radimo_fops);
	if (res) {
		MSG(RADIMO_ERROR, "couldn't register block device\n");
		return res;
	}
	
	/* for media change */
	radimo_changed = 0;
	init_timer(&radimo_timer);
	
	/* set hard- and soft blocksize */
	hardsect_size[RADIMO_MAJOR] = &radimo_hard;
	blksize_size[RADIMO_MAJOR] = &radimo_soft;
	blk_size[RADIMO_MAJOR] = &radimo_size;
	
	/* define our request function */
	blk_dev[RADIMO_MAJOR].request_fn = &radimo_request;
	read_ahead[RADIMO_MAJOR] = radimo_readahead;
	
	MSG(RADIMO_INFO, "loaded\n");
	MSG(RADIMO_INFO, "sector size of %d, block size of %d, total size = %dKb\n",
					radimo_hard, radimo_soft, radimo_size);
	
	return 0;
}

void cleanup_module(void)
{
	unregister_blkdev(RADIMO_MAJOR, "radimo");
	del_timer(&radimo_timer);

	invalidate_buffers(MKDEV(RADIMO_MAJOR,0));

	/* remove our request function */
	blk_dev[RADIMO_MAJOR].request_fn = 0;
	
#ifndef RADIMO_CHEAT_REQUEST
	vfree(radimo_storage);
#endif	

	MSG(RADIMO_INFO, "unloaded\n");
}	
