#pragma once
#include <Arduino.h>

/**
 * @brief State machine that manages SBC.
*/
namespace App
{
    /**
     * @brief Task that handles most of the hardware and MODBUS communication + heartbeating.
    */
    void task(void *pvParameters __attribute__((unused)));

    /**
     * @brief Get FreeRTOS stack size required to run this task.
    */
    constexpr uint16_t getRequiredStack()
    {
        return 173;
    }
}
