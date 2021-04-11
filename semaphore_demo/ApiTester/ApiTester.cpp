#define LOG_TAG "ApiTester"
#include <utils/Log.h>

#include <fcntl.h>

#include "my_test.h"

using namespace android;
int main(void)
{
    ALOGW("-------------ApiTester begin--------------");

    const char *dev = "/dev/api_test";
    int fd = open(dev, O_RDWR);
    ALOGD("open(%s) with ret_fd = %d, strerror=(%s)", dev, fd, strerror(errno));
    if (fd < 0) {
        ALOGE("open failed! forget to insmod api_test.ko?");
        return -1;
    }

    int val = 0;
    int ret = 0;

    ALOGD("-----------------IOCTL_GET/SET_VAL-----------------------");
    ALOGD("after IOCTL_GET_VAL, val = %d", val);
    ioctl(fd, TEST_GET_VAL, &val);
    ALOGD("after IOCTL_GET_VAL, val = %d", val);
    ALOGD("wait for key in val and SET to kernel...");
    scanf("%d", &val);
    ioctl(fd, TEST_SET_VAL, &val);
    val = 0;
    ioctl(fd, TEST_GET_VAL, &val);
    ALOGD("then GET from kernel, val=%d", val);

    ALOGD("-----------------IOCTL_SEMA_DOWN/UP-----------------------");
    while (1) {
        ALOGD("looping. key in up(1), down(2) or quit(others)...");
        scanf("%d", &val);
        if (val == 1) {
            ioctl(fd, TEST_SEMA_UP, NULL);
        } else if (val == 2) {
            ALOGD("key in down_type(1-down; 2-interruptible; 3-killable; 4-trylock; others-TimeoutMs)...");
            ret = scanf("%d", &val);
            if ((ret!=1) || (val<0)) {
                ALOGW("error input? for ret=%d, val=%d.", ret, val);
                break;
            }
            ALOGD("begin to down_type(%d)!", val);
            ret = ioctl(fd, TEST_SEMA_DOWN, &val);
            ALOGD("finish down! DOWN_ret=%d", ret);
            ret = 0;
        } else {
            ALOGW("usr want to quit semaphore test! val=%d", val);
            break;
        }
    }

    ALOGD("-----------------IOCTL_INSTANCE_CNT-----------------------");
    ret = ioctl(fd, TEST_INSTANCE_CNT, &val);
    ALOGD("current dev instance cnt=%d, ret=%d", val, ret);

    ALOGD("-----------------IOCTL_WAIT_EVENT-----------------------");
    ret = ioctl(fd, TEST_WAIT_EVENT, &val);
    ALOGD("dev's task complete=%d, ret=%d", val, ret);

    close(fd);

    ALOGW("-------------ApiTester end--------------");

    return 0;
}
