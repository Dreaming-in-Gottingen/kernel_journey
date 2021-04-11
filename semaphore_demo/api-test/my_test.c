#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <linux/wait.h>

#include "my_test.h"

#define TAG "[api_test]"

#define TEST_MINOR MISC_DYNAMIC_MINOR

#define SEM_TIMEOUT_MS 5000 // user set instead of hard-code

#define DEV_TIMEOUT_MS 2000 // dev should finish work in 200ms

static int gVal = 123456;

// create when open
struct test_client_t {
    int dev_acquired;
    int dev_enabled;

    wait_queue_head_t wait_queue;
};

// init when probe, one device is shared by multi-instances.
struct test_dev_t {
    char *name;
    void *context;
    struct semaphore mutex;
};

static struct test_dev_t g_test_dev;

static atomic_t g_instance_cnt = ATOMIC_INIT(0); // client count of dev

static int test_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    struct test_client_t *client = kmalloc(sizeof(struct test_client_t), GFP_KERNEL);
    int instance_cnt = atomic_read(&g_instance_cnt);

    printk(KERN_WARNING TAG "(%s,%d) (inode=%p, filp=%p, client=%p) current g_instance_cnt=%d, will inc\n", __func__, __LINE__, inode, filp, client, instance_cnt);
    filp->private_data = client;
    /* do something client init */
    init_waitqueue_head(&client->wait_queue);
    atomic_inc(&g_instance_cnt); // return void
    //ret = atomic_inc_return(&g_instance_cnt); // return current val, open will fail! why???

    return ret;
}

static int test_release(struct inode *inode, struct file *filp)
{
    int ret = 0;
    int instance_cnt = atomic_read(&g_instance_cnt);

    printk(KERN_EMERG TAG "(%s,%d) (%p,%p) current g_instance_cnt=%d, will dec\n", __func__, __LINE__, inode, filp, instance_cnt);
    /* do something client deinit */
    atomic_dec(&g_instance_cnt);
    kfree(filp->private_data);
    filp->private_data = NULL;

    return ret;
}

static long test_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int usr_val = 0;
    struct test_client_t *client = filp->private_data;

    //printk("(%s,%d) (filp=%p, cmd=%#x, arg=%#x)", __func__, __LINE__, filp, (int)cmd, (int)arg);
    switch (cmd) {
        case TEST_GET_VAL:
            put_user(gVal, (int __user*)arg);
            printk(KERN_EMERG TAG "--GET_VAL--gVal=%d--\n", gVal);
            break;
        case TEST_SET_VAL:
            get_user(gVal, (int __user*)arg);
            printk(KERN_EMERG TAG "--SET_VAL--gVal=%d--\n", gVal);
            break;
        case TEST_SEMA_UP:
            printk(KERN_WARNING TAG "--SEMA_UP--\n");
            up(&g_test_dev.mutex);
            break;
        case TEST_SEMA_DOWN:
            get_user(usr_val, (int __user*)arg);
            printk(KERN_WARNING TAG "--SEMA_DOWN by type=%d--\n", usr_val);
            // usr_val: use type of down.
            // 1-down; 2-interruptible; 3-killable; 4-trylock; others-timeout ms
            if (usr_val == 1)
                down(&g_test_dev.mutex); // deprecated, use down_interruptible() or down_killable() instead.
            else if (usr_val == 2)
                ret = down_interruptible(&g_test_dev.mutex);
            else if (usr_val == 3)
                ret = down_killable(&g_test_dev.mutex);
            else if (usr_val == 4)
                ret = down_trylock(&g_test_dev.mutex);
            else
                ret = down_timeout(&g_test_dev.mutex, msecs_to_jiffies(usr_val));
            break;
        case TEST_INSTANCE_CNT:
            usr_val = atomic_read(&g_instance_cnt);
            put_user(usr_val, (int __user*)arg);
            printk(KERN_WARNING TAG "--TEST_INSTANCE_CNT--g_instance_cnt=%d--\n", usr_val);
            break;
        case TEST_WAIT_EVENT:
            printk(KERN_WARNING TAG "--TEST_WAIT_EVENT--need to be wake_up or timeout(2s)...--\n");
            ret = wait_event_interruptible_timeout(client->wait_queue, 0, msecs_to_jiffies(DEV_TIMEOUT_MS));
            if (ret == -ERESTARTSYS) {
                printk(KERN_ERR TAG "--TEST_WAIT_EVENT--dev complete with ERESTARTSYS--\n");
                put_user(ret, (int __user*)arg);
                ret = -EINVAL;
            } else {
                printk(KERN_WARNING TAG "--TEST_WAIT_EVENT--dev complete with ret=%d--\n", ret);
                if (ret == 0) {
                    put_user(-ETIMEDOUT, (int __user*)arg);
                    ret = -1;
                } else {
                    ret = 0; // complete task before timeout
                }
            }
            break;
    }
    return ret;
}

static const struct file_operations test_fops = {
    .owner = THIS_MODULE,
    .open = test_open,
    .release = test_release,
    .unlocked_ioctl = test_ioctl,
    .compat_ioctl = NULL,
};

static struct miscdevice test_dev = {
    .minor = TEST_MINOR,
    .name = "api_test",
    .fops = &test_fops,
};

static const struct of_device_id of_match_test_table[] = {
    { .compatible = "xcom,mytest", .data = NULL },
    { },
};

static int test_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct device_node *node = dev->of_node;
    const struct of_device_id *of_id = of_match_node(of_match_test_table, node);

    printk(KERN_WARNING TAG "----------------insmod----------------\n");
    printk(KERN_WARNING TAG "(%s,%d) pdev=%p, dev=%p, node=%p, of_id=%p\n", __func__, __LINE__, pdev, dev, node, of_id);
    sema_init(&g_test_dev.mutex, 1);

    ret = misc_register(&test_dev);
    return ret;
}

static int test_remove(struct platform_device *pdev)
{
    printk(KERN_WARNING TAG "(%s,%d) pdev=%p\n", __func__, __LINE__, pdev);

    misc_deregister(&test_dev);
    printk(KERN_WARNING TAG "----------------rmmod----------------\n");
    return 0;
}

static struct platform_driver test_driver = {
    .probe = test_probe,
    .remove = test_remove,

    .driver = {
        .owner = THIS_MODULE,
        .name = "api_test",
        .of_match_table = of_match_test_table,
    },
};

module_platform_driver(test_driver);

MODULE_DESCRIPTION("KERNEL Common API Test");
MODULE_LICENSE("GPL");
