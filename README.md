# rotary_Device_driver

# 파일 구조
rotary_compelte/
├── Makefile                          # 빌드 스크립트
├── rotary_main.c                     # 메인 모듈 로직 (init/exit, file_operations 구조체 연결)
├── rotary_gpio_defs.h                # 모든 GPIO 핀 번호 및 상수 정의
├── rotary_led_control.c              # LED 제어 함수 구현
├── rotary_led_control.h              # LED 제어 함수 선언
├── rotary_interrupt_handlers.c       # 인터럽트 핸들러 및 타이머 구현
├── rotary_interrupt_handlers.h       # 인터럽트 핸들러 및 타이머 선언
├── rotary_char_dev.c                 # 문자 디바이스 file_operations 함수 구현
└── rotary_char_dev.h                 # 문자 디바이스 관련 선언 및 IOCTL 정의
