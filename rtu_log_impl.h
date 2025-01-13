#pragma once

#include <stdint.h>

#include "drv/tlog.h"

#define RTU_LOG_TP()                                                   TLOG_TP()
#define RTU_LOG_ERROR(str, status)               TLOG_XPRINT8(str, status.value)
#define RTU_LOG_EVENT(str, status)               TLOG_XPRINT8(str, status.value)
#define RTU_LOG_DBG8(str, value)                        TLOG_XPRINT8(str, value)
#define RTU_LOG_DBG16(str, value)                      TLOG_XPRINT16(str, value)
