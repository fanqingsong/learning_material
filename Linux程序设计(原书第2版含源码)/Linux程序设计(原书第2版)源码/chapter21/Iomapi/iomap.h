#define IOMAP_MAJOR		42

#define IOMAP_MAX_DEVS		16

/* define to use readb and writeb to read/write data */
#define IOMAP_BYTE_WISE

/* the device structure */
typedef struct Iomap {
	unsigned long base;
	unsigned long size;
	char *ptr;
} Iomap;

#define IOMAP_GET	_IOR(0xbb, 0, Iomap)
#define IOMAP_SET	_IOW(0xbb, 1, Iomap)
#define IOMAP_CLEAR	_IOW(0xbb, 2, long)

#ifndef DEBUG
#define DEBUG 1
#endif

#ifdef DEBUG
#define MSG(string, args...) printk(KERN_DEBUG "iomap: " string, ##args)
#else
#define MSG(string, args...)
#endif
