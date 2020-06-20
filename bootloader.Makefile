include ../Makefile.defs

# get default linker script for target MCU
# avr-gcc -mmcu=atmega328p -Wl,--verbose

CFLAGS += \
		  -DEEPROM_ADDR_RTU_ADDR=0x0 \
		  -DRTU_ADDR_BASE=0x2000 \
		  -DTLOG_SIZE=256 \
		  -DUSART0_RX_NO_BUFFERING

TARGET = bootloader
CSRCS = \
		../drv/tmr0.c \
		../drv/usart0.c \
		../drv/watchdog.c \
		../modbus-c/atmega328p/rtu_impl.c \
		../modbus-c/crc.c \
		../modbus-c/rtu.c \
		../modbus-c/rtu_memory.c \
		bootloader.c \
		rtu_cmd.c

ifdef RELEASE
	CFLAGS +=  \
		-DASSERT_DISABLE \
		-DTLOG_DISABLE
else
	CSRCS += \
		../drv/tlog.c \
		../hw.c
endif

LDFLAGS += \
		   -Wl,--section-start=.text=$(BOOTLOADER_FLASH_ADDR)

include ../Makefile.rules
