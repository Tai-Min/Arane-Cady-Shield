#pragma once
#include <Arduino.h>

/**
 * @brief MODBUS communication and heartbeat management.
*/
namespace Comm
{
    /**
     * @brief Initialize MODBUS hardware.
    */
    void init();

    /**
     * @bief Poll comm events. Call as frequently as possible.
    */
    void update();

    /**
     * @brief Returns whether MCU is connected to SBC via heartbeating.
    */
    bool connected();

    /**
     * @brief Reset connection status to disconnected.
     */
    void disconnect();

    /**
     * @brief Clear shutdown flag.
     */
    void clearShutdownFlag();
}
