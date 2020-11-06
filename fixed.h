#pragma once

#include <stdint.h>

/* this file should contain only variables not initialized by C runtime
 * this enables preservation of state across System Resets */

#define FIXED_SIZE 16

#define RESET_SIGNATURE_BOOT_APP UINT8_C(0xAA)

typedef union
{

    struct
    {
        uint8_t mcusr;
        uint8_t reset_signature;
        uint8_t reset_counter;
        uint8_t reserved[FIXED_SIZE - 3];
    };

    uint8_t bytes[FIXED_SIZE];
} fixed_t;

extern fixed_t fixed__;
