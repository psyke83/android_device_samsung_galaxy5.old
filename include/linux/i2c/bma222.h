#ifndef __BMA222_H__
#define __BMA222_H__

/* Note: this is not the full kernel header */

#define BMA222_IOC_MAGIC 'B'

#define BMA222_READ_ACCEL_XYZ			_IOWR(BMA222_IOC_MAGIC,46,signed char)

#endif
