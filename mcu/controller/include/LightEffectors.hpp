#pragma once

#include <Arduino.h>

/**
 * Light controller for a pair of LEDs.
*/
class LightEffectors
{

public:
    enum class Effect
    {
        OFF,
        BLINKING,
        MANUAL,
        ALTERNATING_BLINKING,
        FAST_BLINKING
    };

private:
    const uint8_t m_pin1;
    const uint8_t m_pin2;

    uint8_t m_pwmEffectVal = 0; // For automatic effects.

    uint8_t m_pwmVal1 = 0; // For manual control.
    uint8_t m_pwmVal2 = 0; // For manual control.

    Effect m_effect = Effect::OFF;

    unsigned long m_effectStamp = 0; // For delta calculation

    bool m_fstScan = true; // Check first update() call to initialize some variables.
    bool m_inverse = false; // True - light fades, false - light rises.

    /**
     * @brief Update PWM based on blinking ratio and delta time.
    */
    void updatePwm(unsigned long delta, float ratio);

public:
    /**
     * @brief Initialize hardware for both LED devices.
     */
    LightEffectors(uint8_t pin1, uint8_t pin2);

    void init();

    /**
     * @brief Update PWM values for both LED devices. Call as frequently as possible.
     */
    void update();

    /**
     * Set brightness for each LED in MANUAL mode.
     */
    void setManualBrightness(uint8_t val1, uint8_t val2)
    {
        m_pwmVal1 = val1;
        m_pwmVal2 = val2;
    }

    /**
     * Set new effect for effector.
     */
    void setEffect(Effect ef)
    {
        // TODO: consider light value to persist if goes from not none
        m_effect = ef;
    }
};