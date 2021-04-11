#ifndef __MY_TEST_H
#define __MY_TEST_H

#include <linux/ioctl.h>

#define XCOM_TEST_IOCTL_MAGIC 'x'

#define TEST_GET_VAL   _IOR(XCOM_TEST_IOCTL_MAGIC, 1, unsigned int)
#define TEST_SET_VAL   _IOW(XCOM_TEST_IOCTL_MAGIC, 2, unsigned int)

#define TEST_SEMA_UP   _IO(XCOM_TEST_IOCTL_MAGIC, 3)
#define TEST_SEMA_DOWN _IO(XCOM_TEST_IOCTL_MAGIC, 4)

#define TEST_INSTANCE_CNT _IOR(XCOM_TEST_IOCTL_MAGIC, 5, unsigned int)

#define TEST_WAIT_EVENT   _IO(XCOM_TEST_IOCTL_MAGIC, 6)

#endif
