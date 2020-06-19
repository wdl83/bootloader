#pragma once

#include <stddef.h>

#include <drv/assert.h>
#include <hw.h>
#include <modbus-c/rtu.h>
#include <modbus-c/rtu_memory.h>

#ifndef RTU_ADDR
#error "Please define RTU_ADDR"
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
    rtu_memory_t rtu_memory;

    union
    {
        struct
        {
            uint8_t flash_page_update : 1;
            uint8_t reboot : 1;
            uint8_t reserved_wflags : 6;
        };
        uint8_t wflags;
    };

    union
    {
        struct
        {
            uint8_t flash_page_updated : 1;
            uint8_t reserved_rflags : 7;
        };

        uint8_t rflags;
    };

    uint8_t flash_page_updated_num;
    uint16_t flash_page_addr;
    flash_page_t flash_page;
    char tlog[TLOG_SIZE];
} rtu_memory_fields_t;

STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, wflags, sizeof(rtu_memory_t) + 0);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, rflags, sizeof(rtu_memory_t) + 1);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page_updated_num, sizeof(rtu_memory_t) + 2);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page_addr, sizeof(rtu_memory_t) + 3);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, flash_page, sizeof(rtu_memory_t) + 5);
STATIC_ASSERT_STRUCT_OFFSET(rtu_memory_fields_t, tlog, sizeof(rtu_memory_t) + 133);

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
