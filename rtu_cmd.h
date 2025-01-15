#pragma once

#include <stddef.h>
// avr drv
#include "drv/assert.h"
#include "drv/util.h"
#include "hw.h"
// modbus_c
#include "rtu.h"
#include "rtu_memory.h"


typedef union
{
    uint16_t words[FLASH_PAGE_SIZE_IN_WORDS];
    uint8_t bytes[FLASH_PAGE_SIZE_IN_BYTES];
} flash_page_t;

typedef struct
{
    /* begin: private memory (not accessible via rtu_memory_t *) */
    struct
    {
        modbus_rtu_addr_t self_addr;
    } priv;
    /* end: private memory */
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

enum
{
    RTU_MEM_OFFSET =
        sizeof_field(rtu_memory_fields_t, priv)
        + sizeof_field(rtu_memory_fields_t, header)
};

#define VALIDATE_RTU_MEM_OFFSET(field, offset) \
    STATIC_ASSERT_STRUCT_OFFSET( \
        rtu_memory_fields_t, \
        field, \
        sizeof_field(rtu_memory_fields_t, priv) \
        + sizeof_field(rtu_memory_fields_t, header) \
        + offset)

VALIDATE_RTU_MEM_OFFSET(wflags, 0);
VALIDATE_RTU_MEM_OFFSET(rflags, 1);
VALIDATE_RTU_MEM_OFFSET(flash_page_wr_num, 2);
VALIDATE_RTU_MEM_OFFSET(flash_page_rd_num, 4);
VALIDATE_RTU_MEM_OFFSET(flash_page_addr, 6);
VALIDATE_RTU_MEM_OFFSET(flash_page, 8);
VALIDATE_RTU_MEM_OFFSET(eeprom_wr_num, 136);
VALIDATE_RTU_MEM_OFFSET(eeprom_rd_num, 138);
VALIDATE_RTU_MEM_OFFSET(eeprom_addr, 140);
VALIDATE_RTU_MEM_OFFSET(eeprom_data, 142);
VALIDATE_RTU_MEM_OFFSET(mcusr, 143);
VALIDATE_RTU_MEM_OFFSET(tlog, 144);

void rtu_memory_fields_clear(rtu_memory_fields_t *);
void rtu_memory_fields_init(rtu_memory_fields_t *);

uint8_t *rtu_pdu_cb(
    modbus_rtu_state_t *state,
    modbus_rtu_addr_t addr,
    modbus_rtu_fcode_t fcode,
    const uint8_t *begin, const uint8_t *end, const uint8_t *curr,
    uint8_t *dst_begin, const uint8_t *const dst_end,
    uintptr_t user_data);
