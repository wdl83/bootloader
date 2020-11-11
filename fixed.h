#pragma once

#include <stdint.h>

/* this file should contain only variables not initialized by C runtime
 * this enables preservation of state across System Resets */

#define FIXED_SIZE 16

#define RESET_SIGNATURE_BOOT_APP UINT8_C(0xAA)

#define RESET_CODE_POR                                             UINT8_C(0x00)
#define RESET_CODE_BOOTLOADER_IDLE                                 UINT8_C(0x01)
#define RESET_CODE_RTU_ERROR                                       UINT8_C(0x02)
#define RESET_CODE_REBOOT                                          UINT8_C(0x03)
#define RESET_CODE_APP_EXEC_FAILED                                 UINT8_C(0x04)
#define RESET_CODE_APP_OFFSET                                      UINT8_C(0x08)
#define RESET_CODE_APP_IDLE                RESET_CODE_APP_OFFSET + UINT8_C(0x01)

typedef union
{
    struct
    {
        uint8_t last : 4;
        uint8_t curr : 4;
    };
    uint8_t value;
} reset_code_t;

typedef union
{

    struct
    {
        uint8_t mcusr;
        uint8_t reset_signature;
        uint8_t reset_counter;
        uint8_t panic_counter;
        uint8_t app_counter;
        reset_code_t bootloader_reset_code;
        reset_code_t app_reset_code;
        uint8_t reserved[FIXED_SIZE - 7];
    };

    uint8_t bytes[FIXED_SIZE];
} fixed_t;

extern volatile fixed_t fixed__;
