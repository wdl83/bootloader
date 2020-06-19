#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <drv/assert.h>
#include <drv/tlog.h>
#include <drv/watchdog.h>

#include <modbus-c/atmega328p/rtu_impl.h>
#include <modbus-c/rtu.h>

#include "rtu_cmd.h"

/* this variable is not initialized by C runtime
 * this enabled to preserve its state across System Resets */
uint8_t reset_signature_ __attribute__((section(".noinit")));
#define RESET_SIGNATURE_BOOT_APP UINT8_C(0xAA)
/*-----------------------------------------------------------------------------*/
void flash_page_erase(rtu_memory_fields_t *rtu_memory_fields)
{
    boot_page_erase_safe(rtu_memory_fields->flash_page_addr);
}

void flash_page_fill(rtu_memory_fields_t *rtu_memory_fields)
{
    uint16_t addr = rtu_memory_fields->flash_page_addr;

    for(uint8_t i = 0; i < FLASH_PAGE_SIZE_IN_WORDS; ++i)
    {
        boot_page_fill_safe(addr, rtu_memory_fields->flash_page.words[i]);
        addr += sizeof(uint16_t);
    }
}

void flash_page_write(rtu_memory_fields_t *rtu_memory_fields)
{
    boot_page_write_safe(rtu_memory_fields->flash_page_addr);
    boot_spm_busy_wait();
}
/*-----------------------------------------------------------------------------*/
static
void dispatch_uninterruptible(rtu_memory_fields_t *rtu_memory_fields)
{
    if(rtu_memory_fields->reboot)
    {
        rtu_memory_fields->reboot = 0;
        reset_signature_ = RESET_SIGNATURE_BOOT_APP;
        watchdog_enable(WATCHDOG_TIMEOUT_16ms);
        for(;;) {/* wait until reset */}
    }

    if(rtu_memory_fields->flash_page_update)
    {
        rtu_memory_fields->flash_page_update = 0;
        flash_page_fill(rtu_memory_fields);
        flash_page_erase(rtu_memory_fields);
        flash_page_write(rtu_memory_fields);
        ++rtu_memory_fields->flash_page_updated_num;
    }
}

static
void dispatch_interruptible(rtu_memory_fields_t *rtu_memory_fields)
{
}
/*-----------------------------------------------------------------------------*/
__attribute__((noreturn))
void exec_bootloader_code(void)
{
    rtu_memory_fields_t rtu_memory_fields;
    modbus_rtu_state_t state;

    rtu_memory_fields_clear(&rtu_memory_fields);
    rtu_memory_fields_init(&rtu_memory_fields);
    TLOG_INIT(rtu_memory_fields.tlog);

    TLOG_PRINTF("RS%02" PRIX8, reset_signature_);

    modbus_rtu_impl(
        &state,
        NULL /* suspend */,
        NULL /* resume */,
        rtu_pdu_cb,
        (uintptr_t)&rtu_memory_fields);

    /* set SMCR SE (Sleep Enable bit) */
    sleep_enable();

    for(;;)
    {
        cli(); // disable interrupts
        modbus_rtu_event(&state);
        const bool is_idle = modbus_rtu_idle(&state);
        if(is_idle) dispatch_uninterruptible(&rtu_memory_fields);
        sei(); // enabled interrupts
        if(is_idle) dispatch_interruptible(&rtu_memory_fields);
        sleep_cpu();
    }
}

__attribute__((noreturn))
void exec_app_code(void)
{
    asm("jmp 0000");
    for(;;) {}
}

 __attribute__((noreturn))
void main(void)
{
    /* if System Reset was caused by watchdog - WDRE in MCUSR
     * will re-enable watchdog - so must be disabled to
     * avoid endless watchdog System Reset loop
     * (because AVR does not have dedicated System Reset instruction
     * watchdog is used for reset) */
    watchdog_disable();

    if(RESET_SIGNATURE_BOOT_APP == reset_signature_)
    {
        /* reset_signature_ will be overwritten by app code so its state
           after app execution is undefined */
        reset_signature_ = 0;
        exec_app_code();
    }
    else
    {
        reset_signature_ = 0xFF;

        /* map interrupt vector table to bootloader flash */
        {
            MCUCR = M1(IVCE);
            MCUCR = M1(IVSEL);
        }

        exec_bootloader_code();
    }
}
/*-----------------------------------------------------------------------------*/
