#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h> // gpio_request_one, gpio_free, gpio_set_value, ARRAY_SIZE

// 새로 만든 헤더 파일들을 포함합니다.
#include "rotary_gpio_defs.h"
#include "rotary_led_control.h"
#include "rotary_interrupt_handlers.h"
#include "rotary_char_dev.h" // DEVICE_NAME, dev_num, rotary_cdev, rotary_class, rotary_device 등을 사용하기 위해

// 모듈 메타데이터 (기존과 동일)
MODULE_LICENSE("GPL");
MODULE_AUTHOR("KJY");
MODULE_DESCRIPTION("Customized Rotary Encoder Driver");
MODULE_VERSION("final-custom");

// 이 메인 파일에서만 사용될 수 있도록 static const로 선언
static const struct gpio leds[] = {
    { LED0_GPIO, GPIOF_OUT_INIT_LOW, "led0" }, { LED1_GPIO, GPIOF_OUT_INIT_LOW, "led1" },
    { LED2_GPIO, GPIOF_OUT_INIT_LOW, "led2" }, { LED3_GPIO, GPIOF_OUT_INIT_LOW, "led3" },
    { LED4_GPIO, GPIOF_OUT_INIT_LOW, "led4" }, { LED5_GPIO, GPIOF_OUT_INIT_LOW, "led5" },
    { LED6_GPIO, GPIOF_OUT_INIT_LOW, "led6" }, { LED7_GPIO, GPIOF_OUT_INIT_LOW, "led7" },
};
static const struct gpio inputs[] = {
    { ROTARY_S1, GPIOF_IN, "r_clk" }, { ROTARY_S2,  GPIOF_IN, "r_dt"  },
    { ROTARY_KEY, GPIOF_IN, "r_sw"  },
};


// --- 모듈 초기화 함수 ---
static int __init rotary_init(void) {
    int ret = 0;
    int i;

    printk(KERN_INFO "rotary_drv: Initializing driver...\n");

    // 1. 문자 디바이스 등록
    ret = register_rotary_char_device();
    if (ret < 0) {
        printk(KERN_ERR "rotary_drv: Failed to register char device\n");
        return ret;
    }

    // 2. GPIO 핀 요청 및 초기화
    for (i = 0; i < ARRAY_SIZE(leds); i++) {
        ret = gpio_request_one(leds[i].gpio, leds[i].flags, leds[i].label);
        if (ret) {
            printk(KERN_ERR "rotary_drv: Failed to request LED GPIO %d\n", leds[i].gpio);
            // 실패한 GPIO까지는 free
            for (i--; i >= 0; i--) gpio_free(leds[i].gpio);
            goto err_gpio_led;
        }
    }
    for (i = 0; i < ARRAY_SIZE(inputs); i++) {
        ret = gpio_request_one(inputs[i].gpio, inputs[i].flags, inputs[i].label);
        if (ret) {
            printk(KERN_ERR "rotary_drv: Failed to request input GPIO %d\n", inputs[i].gpio);
            // 실패한 GPIO까지는 free
            for (i--; i >= 0; i--) gpio_free(inputs[i].gpio);
            goto err_gpio_input;
        }
    }

    // 3. LED 초기 상태 설정
    update_binary_display(0);
    update_direction_leds(0);
    gpio_set_value(LED5_GPIO, 0);

    // 4. 인터럽트 및 타이머 설정
    ret = setup_rotary_interrupts();
    if (ret < 0) {
        printk(KERN_ERR "rotary_drv: Failed to setup interrupts\n");
        goto err_setup_irqs;
    }

    printk(KERN_INFO "rotary_drv: Driver loaded. Major: %d\n", MAJOR(dev_num));
    return 0; // 초기화 성공

// --- 에러 처리 및 자원 해제 (역순) ---
err_setup_irqs:
    for (i = 0; i < ARRAY_SIZE(inputs); i++) gpio_free(inputs[i].gpio);
err_gpio_input:
    for (i = 0; i < ARRAY_SIZE(leds); i++) gpio_free(leds[i].gpio);
err_gpio_led:
    unregister_rotary_char_device();
    return ret;
}

// --- 모듈 종료 함수 ---
static void __exit rotary_exit(void) {
    int i;

    printk(KERN_INFO "rotary_drv: Unloading driver...\n");

    // 1. 타이머 및 인터럽트 해제
    cleanup_rotary_interrupts();

    // 2. GPIO 핀 해제
    for (i = 0; i < ARRAY_SIZE(inputs); i++) gpio_free(inputs[i].gpio);
    for (i = 0; i < ARRAY_SIZE(leds); i++) {
        gpio_set_value(leds[i].gpio, 0); // 모듈 언로드 시 LED 모두 끄기
        gpio_free(leds[i].gpio);
    }

    // 3. 문자 디바이스 관련 자원 해제
    unregister_rotary_char_device();

    printk(KERN_INFO "rotary_drv: Driver unloaded.\n");
}

// 모듈 초기화/종료 함수 등록
module_init(rotary_init);
module_exit(rotary_exit);
