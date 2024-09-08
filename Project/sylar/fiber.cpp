/**
 * @file fiber.cpp
 * @brief 协程实现
 */

#include <atomic>
#include "fiber.h"
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"

namespace sylar
{
    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
}
