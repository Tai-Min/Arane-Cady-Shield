#pragma once
#include <Arduino.h>

/**
 * @brief Simple SBC control.
 */
class SBC
{
    bool OFF = false;
    bool ON = true;

    const uint8_t m_pin;

public:
    /**
     * @brief Initialize hardware to control SBC.
     *
     * @param pin Pin to use.
     * @param activeLow Whether digital 0 means the SBC is on and 1 that it's OFF.
     */
    SBC(uint8_t pin, bool activeLow) : m_pin(pin)
    {
        if (activeLow)
        {
            OFF = true;
            ON = false;
        }
    }

    void init()
    {
        pinMode(m_pin, OUTPUT);

        off();
    }

    void on()
    {
        digitalWrite(m_pin, ON);
    }

    void off()
    {
        digitalWrite(m_pin, OFF);
    }
};