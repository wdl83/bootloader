DRV_DIR=../atmega328p_drv

CPPFLAGS += -I..
CPPFLAGS += -I$(DRV_DIR)

BOOTLOADER_FLASH_ADDR=0x7000

include $(DRV_DIR)/Makefile.defs

# get default linker script for target MCU
# avr-gcc -mmcu=atmega328p -Wl,--verbose
# hfuse for atmega328p 0x90

CFLAGS += \
		  -DEEPROM_ADDR_RTU_ADDR=0x0 \
		  -DMODBUS_RTU_MEMORY_RD_HOLDING_REGISTERS_DISABLED \
		  -DMODBUS_RTU_MEMORY_WR_REGISTERS_DISABLED \
		  -DMODBUS_RTU_MEMORY_WR_REGISTER_DISABLED \
		  -DRTU_ADDR_BASE=0x2000 \
		  -DTLOG_SIZE=256 \
		  -DUSART0_RX_NO_BUFFERING

TARGET = bootloader
CSRCS = \
		$(DRV_DIR)/drv/tmr0.c \
		$(DRV_DIR)/drv/usart0.c \
		$(DRV_DIR)/drv/watchdog.c \
		../modbus_c/atmega328p/rtu_impl.c \
		../modbus_c/crc.c \
		../modbus_c/rtu.c \
		../modbus_c/rtu_memory.c \
		bootloader.c \
		fixed.c \
		rtu_cmd.c

LDFLAGS += \
		   -Wl,-T atmega328p.ld

ifdef RELEASE
	CFLAGS +=  \
			   -DASSERT_DISABLE \
			   -DTLOG_DISABLE
	LDFLAGS += \
			   -Wl,--section-start=.text=$(BOOTLOADER_FLASH_ADDR)
else
	CSRCS += \
			 $(DRV_DIR)/drv/tlog.c \
			 $(DRV_DIR)/drv/util.c \
			 $(DRV_DIR)/hw.c
endif

include $(DRV_DIR)/Makefile.rules
