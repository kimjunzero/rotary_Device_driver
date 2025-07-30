#ifndef _ROTARY_CHAR_DEV_H_
#define _ROTARY_CHAR_DEV_H_

#include <linux/fs.h>   // struct file_operations를 위해
#include <linux/cdev.h> // dev_t, struct cdev를 위해
#include <linux/device.h> // struct class, struct device를 위해

// IOCTL 명령 정의
#define IOCTL_ROTARY_GET_COUNT _IOR('R', 1, int)
#define IOCTL_ROTARY_SET_COUNT _IOW('R', 2, int)

// 디바이스 이름 정의
#define DEVICE_NAME "rotary_dev"

// 다른 파일에서 접근할 수 있도록 extern 선언
extern dev_t dev_num;
extern struct cdev rotary_cdev;
extern struct class *rotary_class;
extern struct device *rotary_device;
extern const struct file_operations rotary_fops; // file_operations 구조체

// 문자 디바이스 등록/해제 함수 선언
int register_rotary_char_device(void);
void unregister_rotary_char_device(void);

#endif // _ROTARY_CHAR_DEV_H_
