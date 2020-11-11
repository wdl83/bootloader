#include <string.h>

#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include <drv/assert.h>
#include <drv/tlog.h>
#include <drv/watchdog.h>

#include <modbus_c/atmega328p/rtu_impl.h>
#include <modbus_c/rtu.h>

#include "rtu_cmd.h"
#include "fixed.h"

/*-----------------------------------------------------------------------------*/
static
void flash_page_erase(rtu_memory_fields_t *rtu_memory_fields)
{
    boot_page_erase_safe(rtu_memory_fields->flash_page_addr);
}

static
void flash_page_fill(rtu_memory_fields_t *rtu_memory_fields)
{
    uint16_t addr = rtu_memory_fields->flash_page_addr;

    for(uint8_t i = 0; i < FLASH_PAGE_SIZE_IN_WORDS; ++i)
    {
        boot_page_fill_safe(addr, rtu_memory_fields->flash_page.words[i]);
        addr += sizeof(uint16_t);
    }
}

static
void flash_page_write(rtu_memory_fields_t *rtu_memory_fields)
{
    boot_page_write_safe(rtu_memory_fields->flash_page_addr);
    boot_spm_busy_wait();
}
/*-----------------------------------------------------------------------------*/
static
void handle_reboot(rtu_memory_fields_t *rtu_memory_fields)
{
    if(!rtu_memory_fields->reboot) return;

    rtu_memory_fields->reboot = 0;
    fixed__.bootloader_reset_code.curr = RESET_CODE_REBOOT;
    watchdog_enable(WATCHDOG_TIMEOUT_250ms);
    sei(); /* USART0 async transmission in progress - Modbus reply */
    for(;;) {/* wait until reset */}
}

static
void handle_watchdog(rtu_memory_fields_t *rtu_memory_fields)
{
    if(rtu_memory_fields->watchdog_reset)
    {
        rtu_memory_fields->watchdog_reset = 0;
        watchdog_reset();
    }

    if(rtu_memory_fields->watchdog_disable)
    {
        rtu_memory_fields->watchdog_disable = 0;

        if(!rtu_memory_fields->watchdog_disabled)
        {
            watchdog_disable();
            rtu_memory_fields->watchdog_disabled = 1;
        }
    }
}

static
void handle_flash(rtu_memory_fields_t *rtu_memory_fields)
{
    if(!rtu_memory_fields->flash_page_update) return;

    rtu_memory_fields->flash_page_update = 0;

    if(rtu_memory_fields->flash_page_rnw)
    {
        memcpy_P(
            rtu_memory_fields->flash_page.bytes,
            (uint16_t *)rtu_memory_fields->flash_page_addr,
            FLASH_PAGE_SIZE_IN_BYTES);
        ++rtu_memory_fields->flash_page_rd_num;
    }
    else
    {
        flash_page_fill(rtu_memory_fields);
        flash_page_erase(rtu_memory_fields);
        flash_page_write(rtu_memory_fields);
        ++rtu_memory_fields->flash_page_wr_num;
    }
}

static
void handle_eeprom(rtu_memory_fields_t *rtu_memory_fields)
{
    if(!rtu_memory_fields->eeprom_update) return;

    rtu_memory_fields->eeprom_update = 0;

    if(rtu_memory_fields->eeprom_rnw)
    {
        rtu_memory_fields->eeprom_data =
            eeprom_read_byte(
                (const uint8_t *)rtu_memory_fields->eeprom_addr);
        ++rtu_memory_fields->eeprom_rd_num;
    }
    else
    {
        eeprom_write_byte(
            (uint8_t *)rtu_memory_fields->eeprom_addr,
            rtu_memory_fields->eeprom_data);
        ++rtu_memory_fields->eeprom_wr_num;
    }
}

static
void handle_rtu_state(modbus_rtu_state_t *state)
{
    if(RTU_ERR_REBOOT_THREASHOLD > state->err_cntr) return;
    fixed__.bootloader_reset_code.curr = RESET_CODE_RTU_ERROR;
    watchdog_enable(WATCHDOG_TIMEOUT_16ms);
    for(;;) {/* wait until reset */}
}

static
void dispatch(
    modbus_rtu_state_t *state,
    rtu_memory_fields_t *rtu_memory_fields)
{
    handle_rtu_state(state);
    handle_reboot(rtu_memory_fields);
    handle_watchdog(rtu_memory_fields);
    handle_flash(rtu_memory_fields);
    handle_eeprom(rtu_memory_fields);
}
/*-----------------------------------------------------------------------------*/
__attribute__((noreturn))
void exec_bootloader_code(void)
{
    rtu_memory_fields_t rtu_memory_fields;
    modbus_rtu_state_t state;

    rtu_memory_fields_clear(&rtu_memory_fields);
    rtu_memory_fields_init(&rtu_memory_fields);
    rtu_memory_fields.mcusr = fixed__.mcusr;
    TLOG_INIT(rtu_memory_fields.tlog);

    TLOG_XPRINT2x8("MCUSR|RSTC", fixed__.mcusr, fixed__.reset_counter);
    TLOG_XPRINT2x8("PNC|APPCNT", fixed__.panic_counter, fixed__.app_counter);

    modbus_rtu_impl(
        &state,
        eeprom_read_byte((const uint8_t *)EEPROM_ADDR_RTU_ADDR),
        NULL /* suspend */,
        NULL /* resume */,
        rtu_pdu_cb,
        (uintptr_t)&rtu_memory_fields);

    /* set SMCR SE (Sleep Enable bit) */
    sleep_enable();
    fixed__.bootloader_reset_code.curr = RESET_CODE_BOOTLOADER_IDLE;
    watchdog_enable(WATCHDOG_TIMEOUT_8000ms);

    for(;;)
    {
        cli(); // disable interrupts
        modbus_rtu_event(&state);
        const bool is_idle = modbus_rtu_idle(&state);
        if(is_idle) dispatch(&state, &rtu_memory_fields);
        sei(); // enabled interrupts
        sleep_cpu();
    }
}

__attribute__((noreturn))
void exec_app_code(void)
{
    ++fixed__.app_counter;
    fixed__.app_reset_code.last = fixed__.app_reset_code.curr;
    fixed__.app_reset_code.curr = RESET_CODE_APP_EXEC_FAILED;
    watchdog_enable(WATCHDOG_TIMEOUT_16ms);
    asm("jmp 0000");
    for(;;) {}
}

 __attribute__((noreturn))
void main(void)
{
    fixed__.mcusr = MCUSR;

    MCUSR &= ~M4(WDRF, BORF, EXTRF, PORF);

    /* if System Reset was caused by watchdog - WDRE bit in MCUSR
     * will re-enable watchdog - so watchdog must be disabled to
     * avoid endless watchdog System Reset loops
     * (because AVR does not have dedicated System Reset instruction
     * watchdog is used for reset) */
    watchdog_disable();

    if(fixed__.mcusr & M1(PORF))
    {
        /* if power was lost SRAM state is undefined
         * (memset/bzero not used because of volatile)*/
        for(uint8_t i = 0; i < FIXED_SIZE; ++i) fixed__.bytes[i] = 0;
    }

    ++fixed__.reset_counter;
    fixed__.bootloader_reset_code.last = fixed__.bootloader_reset_code.curr;

    /* jump to app code ONLY if reset was caused by watchdog and signature is
     * matching */
    if(
        (fixed__.mcusr & M1(WDRF))
        && RESET_SIGNATURE_BOOT_APP == fixed__.reset_signature)
    {
        fixed__.reset_signature = (uint8_t)~RESET_SIGNATURE_BOOT_APP;
        exec_app_code();
    }
    else
    {
        fixed__.reset_signature = RESET_SIGNATURE_BOOT_APP;

        /* map interrupt vector table to bootloader flash */
        {
            MCUCR = M1(IVCE);
            MCUCR = M1(IVSEL);
        }

        exec_bootloader_code();
    }
}
/*-----------------------------------------------------------------------------*/
