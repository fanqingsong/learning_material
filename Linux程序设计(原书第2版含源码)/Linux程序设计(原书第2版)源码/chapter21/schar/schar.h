#define SCHAR_MAJOR		42

#define SCHAR_INC		1024
#define SCHAR_TIMER_DELAY	5*HZ
#define SCHAR_POOL_MIN		10*1024

#define SCHAR_MAX_SYSCTL	512
#define DEV_SCHAR		10
#define DEV_SCHAR_ENTRY		1

/* ioctl's for schar. */
#define SCHAR_IOCTL_BASE	0xbb
#define SCHAR_TOGGLE_DEBUG	_IO(SCHAR_IOCTL_BASE, 0)
#define SCHAR_GET_POOL		_IOR(SCHAR_IOCTL_BASE, 1, unsigned long)
#define SCHAR_GET_TIMER_FIRES	_IOR(SCHAR_IOCTL_BASE, 2, unsigned long)
#define SCHAR_GET_READ		_IOR(SCHAR_IOCTL_BASE, 3, unsigned long)
#define SCHAR_GET_WRITTEN	_IOR(SCHAR_IOCTL_BASE, 4, unsigned long)
#define SCHAR_EX_TIMER_DELAY	_IOWR(SCHAR_IOCTL_BASE, 5, unsigned long)
#define SCHAR_EX_POOL_MIN	_IOWR(SCHAR_IOCTL_BASE, 6, unsigned)
#define SCHAR_EX_TIMER_INC	_IOWR(SCHAR_IOCTL_BASE, 7, unsigned long)


#define DEBUG

#ifdef DEBUG
#define MSG(string, args...) if (schar_debug) printk(KERN_DEBUG "schar: " string, ##args)
#else
#define MSG(string, args...)
#endif
