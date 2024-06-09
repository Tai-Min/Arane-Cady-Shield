#pragma once
#include <Arduino.h>

/**
 * @brief Simple GPIO button.
*/
class Button
{
    bool RELEASED = false;
    bool PRESSED = true;

    const uint8_t m_pin;

    bool m_previousState = false;
    bool m_clicked = false;
    bool m_longPressed = false;
    bool m_fstScan = true;
    unsigned long m_pressedStartStamp = 0;

public:
    /**
     * @brief Class constructor.
     * 
     * @param pin GPIO to use
     * @param activeLow Whether digital 0 means that button is pressed.
    */
    Button(uint8_t pin, bool activeLow);

    void init();

    /**
     * @brief Update internal state. Call as frequently as possible.
    */
    void update();

    /**
     * @brief Whether button is pressed.
    */
    bool pressed();

    /**
     * @brief Whether button is long pressed.
    */
    bool longPressed();

    /**
     * @brief Whether click occured.
    */
    bool clicked();

    /**
     * @brief Clear any events that occured. Use each time event was read.
    */
    void clearState();
};