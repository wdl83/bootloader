#include <string.h>
#include <stddef.h>

#include <drv/tlog.h>

#include "rtu_cmd.h"

void rtu_memory_fields_clear(rtu_memory_fields_t *fields)
{
    memset(fields, 0, sizeof(rtu_memory_fields_t));
}

void rtu_memory_fields_init(rtu_memory_fields_t *fields)
{
    fields->header.addr_begin = RTU_ADDR_BASE;
    fields->header.addr_end = RTU_ADDR_BASE + sizeof(rtu_memory_fields_t) - sizeof(rtu_memory_header_t);
}

uint8_t *rtu_pdu_cb(
    modbus_rtu_state_t *state,
    modbus_rtu_addr_t addr,
    modbus_rtu_fcode_t fcode,
    const uint8_t *begin, const uint8_t *end,
    /* curr == begin + sizeof(addr_t) + sizeof(fcode_t) */
    const uint8_t *curr,
    uint8_t *dst_begin, const uint8_t *const dst_end,
    uintptr_t user_data)
{
    rtu_memory_fields_t *rtu_memory_fields = (rtu_memory_fields_t *)user_data;

    //TLOG_XPRINT16("S|F", ((uint16_t)addr << 8) | fcode);

    if(modbus_rtu_addr(state) != addr) goto exit;

    *dst_begin++ = addr;

    dst_begin =
        rtu_memory_pdu_cb(
            (rtu_memory_t *)&rtu_memory_fields->header,
            fcode,
            begin + sizeof(addr), end,
            curr,
            dst_begin, dst_end);
exit:
    return dst_begin;
}
