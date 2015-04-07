#include <asm-generic/ioctl.h>
#define __KERNEL__
struct mutex {};
struct delayed_work {};

#define panic printk
#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) (x)

/**
 * Reuse 'sysimgblt.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../linux/drivers/video/sysimgblt.c"


/**
 * Reuse 'syscopyarea.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../linux/drivers/video/syscopyarea.c"


/**
 * Reuse 'sysfillrect.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../linux/drivers/video/sysfillrect.c"
