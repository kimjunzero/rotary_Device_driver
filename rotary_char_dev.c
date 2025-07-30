#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> // copy_to_user, copy_from_user
#include <linux/device.h>

#include "rotary_gpio_defs.h"    // GPIO 핀 정의 (LED5_GPIO 등)
#include "rotary_led_control.h"  // LED 업데이트 함수 (update_binary_display)
#include "rotary_interrupt_handlers.h" // rotary_count 접근을 위해
#include "rotary_char_dev.h"     // 자신의 헤더 포함

// 전역 변수 정의 (extern 선언과 일치)
dev_t dev_num;
struct cdev rotary_cdev;
struct class *rotary_class;
struct device *rotary_device;

// --- 파일 오퍼레이션 구현 ---
static int rotary_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "rotary_dev: Device opened.\n");
    return 0;
}

static int rotary_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "rotary_dev: Device closed.\n");
    return 0;
}

static ssize_t rotary_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
    int len;
    char kbuf[20];
    if (*pos > 0) return 0;
    len = scnprintf(kbuf, sizeof(kbuf), "%d\n", rotary_count); // rotary_count는 handlers.h에서 extern
    if (copy_to_user(buf, kbuf, len)) return -EFAULT;
    *pos += len; return len;
}

static ssize_t rotary_write(struct file *file, const char __user *buf, size_t count, loff_t *pos) {
    long new_count;
    int ret;
    char kbuf[20];
    if (count >= sizeof(kbuf)) return -EINVAL;
    if (copy_from_user(kbuf, buf, count)) return -EFAULT;
    kbuf[count] = '\0';
    ret = kstrtol(kbuf, 10, &new_count);
    if (ret != 0) return ret;
    rotary_count = new_count; // rotary_count는 handlers.h에서 extern
    update_binary_display(rotary_count); // LED 업데이트
    return count;
}

static long rotary_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    int val;
    switch (cmd) {
        case IOCTL_ROTARY_GET_COUNT:
            val = rotary_count; // rotary_count는 handlers.h에서 extern
            if (copy_to_user((int __user *)arg, &val, sizeof(val))) ret = -EFAULT;
            break;
        case IOCTL_ROTARY_SET_COUNT:
            if (copy_from_user(&val, (int __user *)arg, sizeof(val))) {
                ret = -EFAULT;
            } else {
                rotary_count = val; // rotary_count는 handlers.h에서 extern
                update_binary_display(rotary_count); // LED 업데이트
            }
            break;
        default:
            ret = -ENOTTY;
            break;
    }
    return ret;
}

// 파일 오퍼레이션 구조체 정의 (extern 선언과 일치)
const struct file_operations rotary_fops = {
    .owner           = THIS_MODULE,
    .open            = rotary_open,
    .release         = rotary_release,
    .read            = rotary_read,
    .write           = rotary_write,
    .unlocked_ioctl  = rotary_ioctl,
};

// --- 문자 디바이스 등록 함수 ---
int register_rotary_char_device(void) {
    int ret = 0;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "rotary_dev: Failed to allocate char device region\n");
        return ret;
    }
    cdev_init(&rotary_cdev, &rotary_fops);
    ret = cdev_add(&rotary_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "rotary_dev: Failed to add cdev\n");
        goto err_cdev_add;
    }
    rotary_class = class_create(DEVICE_NAME);
    if (IS_ERR(rotary_class)) {
        ret = PTR_ERR(rotary_class);
        printk(KERN_ERR "rotary_drv: Failed to create class\n");
        goto err_class_create;
    }
    rotary_device = device_create(rotary_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(rotary_device)) {
        ret = PTR_ERR(rotary_device);
        printk(KERN_ERR "rotary_drv: Failed to create device\n");
        goto err_device_create;
    }
    printk(KERN_INFO "rotary_drv: Device %s created successfully (Major: %d, Minor: %d).\n", DEVICE_NAME, MAJOR(dev_num), MINOR(dev_num));
    return 0; // 성공

err_device_create:
    class_destroy(rotary_class);
err_class_create:
    cdev_del(&rotary_cdev);
err_cdev_add:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

// --- 문자 디바이스 해제 함수 ---
void unregister_rotary_char_device(void) {
    device_destroy(rotary_class, dev_num);
    class_destroy(rotary_class);
    cdev_del(&rotary_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "rotary_dev: Device %s unmounted.\n", DEVICE_NAME);
}
