/*
 *	Oops
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

int init_module(void)
{
	printk(KERN_DEBUG "Derefencing null pointer : \n");
	*(int *)0 = 1;
	return 0;
}

void cleanup_module(void)
{
	/* module is stuck */
}
