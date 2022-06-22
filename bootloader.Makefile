DRV = atmega328p_drv
MODBUS_C = modbus_c
OBJ_DIR = obj


BOOTLOADER_FLASH_ADDR=0x7000

include $(DRV)/Makefile.defs

# get default linker script for target MCU
# avr-gcc -mmcu=atmega328p -Wl,--verbose
# hfuse for atmega328p 0x90

CFLAGS += \
	-DEEPROM_ADDR_RTU_ADDR=0x0 \
	-DMODBUS_RTU_MEMORY_RD_HOLDING_REGISTERS_DISABLED \
	-DMODBUS_RTU_MEMORY_WR_REGISTERS_DISABLED \
	-DMODBUS_RTU_MEMORY_WR_REGISTER_DISABLED \
	-DRTU_ADDR_BASE=0x2000 \
	-DRTU_ERR_REBOOT_THREASHOLD=16 \
	-DTLOG_SIZE=256 \
	-DUSART0_RX_NO_BUFFERING \
	-I$(DRV) \
	-I.

TARGET = bootloader
CSRCS = \
		$(DRV)/drv/tmr0.c \
		$(DRV)/drv/usart0.c \
		$(DRV)/drv/watchdog.c \
		$(MODBUS_C)/atmega328p/crc.c \
		$(MODBUS_C)/atmega328p/rtu_impl.c \
		$(MODBUS_C)/rtu.c \
		$(MODBUS_C)/rtu_memory.c \
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
			 $(DRV)/drv/tlog.c \
			 $(DRV)/drv/util.c \
			 $(DRV)/hw.c
endif

include $(DRV)/Makefile.rules

clean:
	rm $(OBJ_DIR) -rf
