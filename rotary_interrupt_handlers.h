#ifndef _ROTARY_INTERRUPT_HANDLERS_H_
#define _ROTARY_INTERRUPT_HANDLERS_H_

#include <linux/interrupt.h> // irqreturn_t를 위해
#include <linux/timer.h>     // struct timer_list를 위해

// 이 파일에서 관리하는 전역 변수들 (다른 파일에서 접근하기 위해 extern 선언)
extern int rotary_count;
extern int led5_state;
extern int rotary_irq_clk;
extern int key_switch_irq;
extern int last_rotary_clk_state;
extern unsigned long last_rotary_irq_time;
extern struct timer_list debounce_timer;

// 인터럽트 핸들러 함수 선언
irqreturn_t rotary_encoder_isr(int irq, void *dev_id);
irqreturn_t key_switch_isr(int irq, void *dev_id);

// 타이머 콜백 함수 선언
void debounce_timer_callback(struct timer_list *t);

// 인터럽트 및 타이머 초기화/정리 함수 선언
int setup_rotary_interrupts(void);
void cleanup_rotary_interrupts(void);

#endif // _ROTARY_INTERRUPT_HANDLERS_H_
