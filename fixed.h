#pragma once

#include <stdint.h>

/* this file should contain only variables not initialized by C runtime
 * this enables preservation of state across System Resets */

#define RESET_SIGNATURE_BOOT_APP UINT8_C(0xAA)

extern uint8_t mcusr__;
extern uint8_t reset_signature__;
