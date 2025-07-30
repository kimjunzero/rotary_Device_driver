obj-m := rotary.o

rotary-objs := rotary_main.o \
               rotary_led_control.o \
               rotary_interrupt_handlers.o \
               rotary_char_dev.o

KDIR := $(HOME)/project/linux
PWD  := $(shell pwd)

all:
	# KDIR의 커널 빌드 시스템을 사용하여 현재 디렉토리(M=$(PWD))의 모듈을 빌드
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	# KDIR의 커널 빌드 시스템을 사용하여 현재 디렉토리(M=$(PWD))의 빌드 결과물 정리
	$(MAKE) -C $(KDIR) M=$(PWD) clean

