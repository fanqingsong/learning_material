#define RADIMO_MAJOR		42

#define RADIMO_TIMER_DELAY	60*HZ

/* msg masks */
#define RADIMO_OPEN		1
#define RADIMO_IOCTL		2
#define RADIMO_INFO		4
#define RADIMO_REQUEST		8
#define RADIMO_TIMER		16
#define RADIMO_ERROR		32

/* set to 1 for rd style requests */
#define RADIMO_CHEAT_REQUEST

#ifndef MSG_MASK
#define MSG_MASK ( RADIMO_IOCTL | RADIMO_INFO | RADIMO_ERROR )
#endif

#define MSG(mask, string, args...) \
	if (MSG_MASK & mask) printk(KERN_DEBUG "radimo: " string, ##args)
