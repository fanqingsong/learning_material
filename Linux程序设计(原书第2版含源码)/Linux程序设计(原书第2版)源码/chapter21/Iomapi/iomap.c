/*
 *	Example of memory mapping i/o memory
 */

#include <linux/module.h>

#if defined (CONFIG_SMP)
#define __SMP__
#endif

#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/wrapper.h>

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/io.h>

#include "iomap.h"

Iomap *iomap_dev[IOMAP_MAX_DEVS];

MODULE_DESCRIPTION("iomap, mapping i/o memory");
MODULE_AUTHOR("Jens Axboe");

int iomap_remove(Iomap *idev)
{
	iounmap(idev->ptr);
	MSG("buffer at 0x%lx removed\n", idev->base);
	return 0;
}

int iomap_setup(Iomap *idev)
{
	/* remap the i/o region */
	idev->ptr = ioremap(idev->base, idev->size);
	MSG("setup: 0x%lx extending 0x%lx bytes\n", idev->base, idev->size);
	return 0;
}

static int iomap_mmap(struct file *file, struct vm_area_struct *vma)
{
	Iomap *idev = iomap_dev[MINOR(file->f_dentry->d_inode->i_rdev)];
	unsigned long size;
	
	/* no such device */
	if (!idev->base)
		return -ENXIO;
	
	/* size must be a multiple of PAGE_SIZE */
	size = vma->vm_end - vma->vm_start;
	if (size % PAGE_SIZE)
		return -EINVAL;
	
	if (remap_page_range(vma->vm_start, idev->base, size, vma->vm_page_prot))
		return -EAGAIN;
	
	MSG("region mmapped\n");
	return 0;
}

static int iomap_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	Iomap *idev = iomap_dev[MINOR(inode->i_rdev)];
	
	switch (cmd) {
		/* create the wanted device */
		case IOMAP_SET: {
			/* if base is set, device is in use */
			if (idev->base)
				return -EBUSY;
			if (copy_from_user(idev, (Iomap *)arg, sizeof(Iomap)))
				return -EFAULT;
			/* base and size must be page aligned */
			if (idev->base % PAGE_SIZE || idev->size % PAGE_SIZE) {
				idev->base = 0;
				return -EINVAL;
			}
			MSG("setting up minor %d\n", MINOR(inode->i_rdev));
			iomap_setup(idev);
			return 0;
		}

		case IOMAP_GET: {
			/* maybe device is not set up */
			if (!idev->base)
				return -ENXIO;
			if (copy_to_user((Iomap *)arg, idev, sizeof(Iomap)))
				return -EFAULT;
			return 0;
		}

		case IOMAP_CLEAR: {
			long tmp;
			/* if base is set, device is in use */
			if (!idev->base)
				return -EBUSY;
			if (get_user(tmp, (long *)arg))
				return -EFAULT;
			memset_io(idev->ptr, tmp, idev->size);
			return 0;
		}
		
	}

	return -ENOTTY;
}

static ssize_t iomap_read(struct file *file, char *buf, size_t count,
			  loff_t *offset)
{
	Iomap *idev = iomap_dev[MINOR(file->f_dentry->d_inode->i_rdev)];
	char *tmp;	
	
	/* device not set up */
	if (!idev->base)
		return -ENXIO;

	/* beyond or at end? */
	if (file->f_pos >= idev->size)
		return 0;

	tmp = (char *) kmalloc(count, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	/* adjust access beyond end */
	if (file->f_pos + count > idev->size)
		count = idev->size - file->f_pos;

	/* get the data from the mapped region */
#ifdef IOMAP_BYTE_WISE
	{ int i;
	for (i = 0; i < count; i++)
		tmp[i] = readb(idev->ptr+file->f_pos+i);
	}
#else
	memcpy_fromio(tmp, idev->ptr+file->f_pos, count);
#endif

	/* copy retrieved data back to app */
	if (copy_to_user(buf, tmp, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	file->f_pos += count;
	kfree(tmp);
	return count;
}

static ssize_t iomap_write(struct file *file, const char *buf, size_t count,
			   loff_t *offset)
{
	Iomap *idev = iomap_dev[MINOR(file->f_dentry->d_inode->i_rdev)];
	char *tmp;
	
	/* device not set up */
	if (!idev->base)
		return -ENXIO;

	/* end of mapping? */
	if (file->f_pos >= idev->size)
		return 0;

	tmp = (char *) kmalloc(count, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	/* adjust access beyond end */
	if (file->f_pos + count > idev->size)
		count = idev->size - file->f_pos;

	/* get user data */
	if (copy_from_user(tmp, buf, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	/* write data to i/o region */
#ifdef IOMAP_BYTE_WISE
	{ int i;
	for (i = 0; i < count; i++)
		writeb(tmp[i], idev->ptr + file->f_pos + i);
	}
#else
	memcpy_toio(idev->ptr+file->f_pos, tmp, count);
#endif

	file->f_pos += count;
	kfree(tmp);
	return count;
}

int iomap_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);
	
	/* no such device */
	if (minor >= IOMAP_MAX_DEVS)
		return -ENXIO;

	MOD_INC_USE_COUNT;
	return 0;
}

int iomap_release(struct inode *inode, struct file *file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

struct file_operations iomap_fops = {
	NULL,		/* llseek */
	iomap_read,
	iomap_write,
	NULL,		/* readdir */
	NULL,		/* poll */
	iomap_ioctl,
	iomap_mmap,
	iomap_open,
	NULL,		/* flush */
	iomap_release,
	NULL,		/* fsync */
	NULL,		/* fasync */
	NULL		/* lock */
};

int init_module(void)
{
	int res, i;
	
	/* register device with kernel */
	res = register_chrdev(IOMAP_MAJOR, "iomap", &iomap_fops);
	if (res) {
		MSG("can't register device with kernel\n");
		return res;
	}
	
	for (i = 0; i < IOMAP_MAX_DEVS; i++) {
		iomap_dev[i] = (Iomap *) kmalloc(sizeof(Iomap), GFP_KERNEL);
		iomap_dev[i]->base = 0;
	}
	
	MSG("module loaded\n");
	return 0;
}

void cleanup_module(void)
{
	int i = 0;
	Iomap *tmp;
	
	/* delete the devices */
	for (tmp = iomap_dev[i]; i < IOMAP_MAX_DEVS; tmp = iomap_dev[++i]) {
		if (tmp->base)
			iomap_remove(tmp);
		kfree(tmp);
	}

	unregister_chrdev(IOMAP_MAJOR, "iomap");
	MSG("unloaded\n");
	return;
}
	
