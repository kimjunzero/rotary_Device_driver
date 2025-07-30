#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include "rotary_gpio_defs.h"         // GPIO 핀 정의를 위해
#include "rotary_led_control.h"       // LED 업데이트 함수를 위해
#include "rotary_interrupt_handlers.h" // 자신의 헤더 포함

// 이 파일에서 관리하는 전역 변수들 (extern 선언과 일치)
int rotary_count = 0;
int led5_state = 0;
int rotary_irq_clk;
int key_switch_irq;
int last_rotary_clk_state;
unsigned long last_rotary_irq_time = 0;
struct timer_list debounce_timer;

// --- 로터리 엔코더 인터럽트 서비스 루틴 (ISR) ---
irqreturn_t rotary_encoder_isr(int irq, void *dev_id) {
    unsigned long current_time = jiffies;
    if (time_after(current_time, last_rotary_irq_time + msecs_to_jiffies(DEBOUNCE_TIME_MS/5))) { // 2ms
        int current_clk_state = gpio_get_value(ROTARY_S1);
        if (current_clk_state != last_rotary_clk_state) {
            int dt_state = gpio_get_value(ROTARY_S2);
            if (current_clk_state == 0) { // CLK 핀이 LOW로 떨어질 때
                if (dt_state == 1) { // 시계 방향 회전
                    rotary_count++;
                    update_direction_leds(1);
                    printk(KERN_INFO "Rotary Encoder: Clockwise, count = %d\n", rotary_count);
                } else { // 반시계 방향 회전
                    rotary_count--;
                    update_direction_leds(-1);
                    printk(KERN_INFO "Rotary Encoder: Counter-clockwise, count = %d\n", rotary_count);
                }
                update_binary_display(rotary_count);
            }
        }
        last_rotary_clk_state = current_clk_state;
        last_rotary_irq_time = current_time;
    }
    return IRQ_HANDLED;
}

// --- 키 스위치 디바운스 타이머 콜백 ---
void debounce_timer_callback(struct timer_list *t) {
    int current_state = gpio_get_value(ROTARY_KEY);
    if (current_state == 0) { // 버튼이 눌린 상태(LOW)일 때만 처리
        led5_state = !led5_state;
        gpio_set_value(LED5_GPIO, led5_state);
        printk(KERN_INFO "Key Switch: LED5 toggled to %s\n", led5_state ? "ON" : "OFF");
    }
    enable_irq(key_switch_irq); // 다음 인터럽트 허용
}

// --- 키 스위치 인터럽트 서비스 루틴 (ISR) ---
irqreturn_t key_switch_isr(int irq, void *dev_id) {
    disable_irq_nosync(key_switch_irq); // 인터럽트 비활성화
    mod_timer(&debounce_timer, jiffies + msecs_to_jiffies(DEBOUNCE_TIME_MS)); // 디바운스 타이머 시작
    return IRQ_HANDLED;
}

// --- 인터럽트 및 타이머 설정 함수 ---
int setup_rotary_interrupts(void) {
    int ret = 0;

    // 로터리 CLK 인터럽트 요청
    rotary_irq_clk = gpio_to_irq(ROTARY_S1);
    if (rotary_irq_clk < 0) {
        printk(KERN_ERR "rotary_drv: Failed to get IRQ for ROTARY_S1\n");
        return rotary_irq_clk;
    }
    ret = request_irq(rotary_irq_clk, rotary_encoder_isr,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "rotary_clk_irq", NULL);
    if (ret) {
        printk(KERN_ERR "rotary_drv: Failed to request rotary CLK IRQ\n");
        return ret;
    }

    // 키 스위치 인터럽트 요청
    key_switch_irq = gpio_to_irq(ROTARY_KEY);
    if (key_switch_irq < 0) {
        printk(KERN_ERR "rotary_drv: Failed to get IRQ for ROTARY_KEY\n");
        free_irq(rotary_irq_clk, NULL); // 이전 IRQ 해제
        return key_switch_irq;
    }
    ret = request_irq(key_switch_irq, key_switch_isr,
                      IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                      "key_switch_irq", NULL);
    if (ret) {
        printk(KERN_ERR "rotary_drv: Failed to request key switch IRQ\n");
        free_irq(rotary_irq_clk, NULL); // 이전 IRQ 해제
        return ret;
    }

    // 타이머 초기화 (setup_rotary_interrupts에서 debounce_timer 설정)
    timer_setup(&debounce_timer, debounce_timer_callback, 0);

    last_rotary_clk_state = gpio_get_value(ROTARY_S1); // 초기 상태 저장

    return 0; // 성공
}

// --- 인터럽트 및 타이머 정리 함수 ---
void cleanup_rotary_interrupts(void) {
    del_timer_sync(&debounce_timer); // 실행 중인 타이머 제거
    free_irq(key_switch_irq, NULL);
    free_irq(rotary_irq_clk, NULL);
}
