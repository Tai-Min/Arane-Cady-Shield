#pragma once
#include <Arduino.h>

/**
 * @brief Global state, thread safe.
*/
namespace State
{
    // MCU request shutdown of the SBC
    bool getShutdownRequest();
    // MCU request shutdown of the SBC
    void setShutdownRequest(bool req);

    // SBC informed MCU about shutdown
    bool getShutdownFlag();
    // SBC informed MCU about shutdown
    void setShutdownFlag(bool flag);

    bool getDisplayState();
    void setDisplayState(bool state);

    bool getJoy1Enable();
    void setJoy1Enable(bool ena);

    bool getJoy2Enable();
    void setJoy2Enable(bool ena);

    uint8_t getJoy1Brightness();
    void setJoy1Brightness(uint8_t val);

    uint8_t getJoy2Brightness();
    void setJoy2Brightness(uint8_t val);

    uint8_t getSelectorValue();
    void setSelectorValue(uint8_t val);
}