#include <linux/gpio.h> // gpio_set_value를 위해 필요

#include "rotary_gpio_defs.h"    // GPIO 핀 정의를 위해
#include "rotary_led_control.h"  // 자신의 헤더 포함

// --- LED0~LED4에 0~31 값을 이진수로 표시하는 함수 ---
void update_binary_display(int value) {
    value = value % 32;
    if (value < 0) value += 32;

    gpio_set_value(LED0_GPIO, (value & 0x01) ? 1 : 0);
    gpio_set_value(LED1_GPIO, (value & 0x02) ? 1 : 0);
    gpio_set_value(LED2_GPIO, (value & 0x04) ? 1 : 0);
    gpio_set_value(LED3_GPIO, (value & 0x08) ? 1 : 0);
    gpio_set_value(LED4_GPIO, (value & 0x10) ? 1 : 0);
}

// --- LED6 (반시계), LED7 (시계) 방향 표시 함수 ---
void update_direction_leds(int direction) {
    if (direction == 1) { // 시계 방향
        gpio_set_value(LED7_GPIO, 1); // LED7 ON
        gpio_set_value(LED6_GPIO, 0); // LED6 OFF
    } else if (direction == -1) { // 반시계 방향
        gpio_set_value(LED7_GPIO, 0); // LED7 OFF
        gpio_set_value(LED6_GPIO, 1); // LED6 ON
    } else { // 둘 다 끄기 (초기 상태 또는 비활성화 시)
        gpio_set_value(LED7_GPIO, 0);
        gpio_set_value(LED6_GPIO, 0);
    }
}
