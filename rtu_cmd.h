#pragma once

#include <stddef.h>

#include <drv/assert.h>
#include <hw.h>
#include <modbus_c/rtu.h>
#include <modbus_c/rtu_memory.h>

#ifndef EEPROM_ADDR_RTU_ADDR
#error "Please define EEPROM_ADDR_RTU_ADDR"
#endif

#ifndef RTU_ADDR_BASE
#error "Please define RTU_ADDR_BASE"
#endif

typedef union
{
    uint16_t words[FLASH_PAGE_SIZE_IN_WORDS];
    uint8_t bytes[FLASH_PAGE_SIZE_IN_BYTES];
} flash_page_t;

typedef struct
{
    rtu_memory_header_t header;

    union
    {
        struct
        {
            uint8_t flash_page_update : 1; // LSB
            uint8_t flash_page_rnw: 1;
            uint8_t eeprom_update : 1;
            uint8_t eeprom_rnw : 1;
            uint8_t watchdog_disable : 1;
            uint8_t watchdog_reset : 1;
            uint8_t : 1;
            uint8_t reboot : 1;
        };
        uint8_t wflags;
    };

    union
    {
        struct
        {
            uint8_t watchdog_disabled : 1;
            uint8_t : 7;
        };

        uint8_t rflags;
    };

    uint16_t flash_page_wr_num;
    uint16_t flash_page_rd_num;
    uint16_t flash_page_addr;
    flash_page_t flash_page;

    uint16_t eeprom_wr_num;
    uint16_t eeprom_rd_num;
    uint16_t eeprom_addr;
    uint8_t eeprom_data;
    uint8_t mcusr;

    char tlog[TLOG_SIZE];
} rtu_memory_fields_t;

STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, wflags, sizeof(rtu_memory_header_t) + 0);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, rflags, sizeof(rtu_memory_header_t) + 1);

STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page_wr_num, sizeof(rtu_memory_header_t) + 2);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page_rd_num, sizeof(rtu_memory_header_t) + 4);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page_addr, sizeof(rtu_memory_header_t) + 6);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page, sizeof(rtu_memory_header_t) + 8);

STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, eeprom_wr_num, sizeof(rtu_memory_header_t) + 136);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, eeprom_rd_num, sizeof(rtu_memory_header_t) + 138);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, eeprom_addr, sizeof(rtu_memory_header_t) + 140);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, eeprom_data, sizeof(rtu_memory_header_t) + 142);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, mcusr, sizeof(rtu_memory_header_t) + 143);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, tlog, sizeof(rtu_memory_header_t) + 144);

void rtu_memory_fields_clear(rtu_memory_fields_t *);
void rtu_memory_fields_init(rtu_memory_fields_t *);

uint8_t *rtu_pdu_cb(
    modbus_rtu_state_t *state,
    modbus_rtu_addr_t addr,
    modbus_rtu_fcode_t fcode,
    const uint8_t *begin, const uint8_t *end,
    const uint8_t *curr,
    uint8_t *dst_begin, const uint8_t *const dst_end,
    uintptr_t user_data);
