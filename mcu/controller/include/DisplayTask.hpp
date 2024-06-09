#pragma once
#include <Arduino.h>

/**
 * @brief Task to controll whether the display is on or off.
*/
namespace Disp
{
    /**
     * @brief Run task that controls display hardware via State.
    */
    void task(void *pvParameters __attribute__((unused)));

    /**
     * @brief Get FreeRTOS stack size required to run this task.
    */
    constexpr uint16_t getRequiredStack()
    {
        return 104;
    }

    /**
     * @brief Force display off no matter the state.
    */
    void forceOff();

    /**
     * @brief Remove force Off.
    */
    void removeForceOff();
}