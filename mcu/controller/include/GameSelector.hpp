#pragma once
#include <Arduino.h>
#include "Encoder.hpp"
#include <TM1637Display.h>
#include "Button.hpp"
#include "Log.hpp"
#include "Config.hpp"

class GameSelectorEncoderDisplay
{
private:
    Button m_applyBtn;
    Encoder m_encoder;
    TM1637Display m_display;

    uint8_t m_savedValue = 0;
    const uint8_t m_numSlots;

public:
    GameSelectorEncoderDisplay(uint8_t applyPin, bool applyBtnInverse,
                               uint8_t encoderClickPin, uint8_t encoderDirPin,
                               uint8_t dispClk, uint8_t dispDio, uint8_t numSlots)
        : m_applyBtn(applyPin, applyBtnInverse),
          m_encoder(encoderClickPin, encoderDirPin, Config::GameSelector::ENCODER_DIR_INVERTED, numSlots),
          m_display(dispClk, dispDio),
          m_numSlots(numSlots)
    {
    }

    void init()
    {
        m_applyBtn.init();
        m_encoder.init();
        m_display.setBrightness(0x0f);
    }

    void encoderUpdate()
    {
        m_encoder.update();
    }

    void update()
    {
        uint8_t reading = m_encoder.read();
        m_display.showNumberDec(reading, true);

        m_applyBtn.update();
        
        if (m_applyBtn.clicked())
        {
            m_applyBtn.clearState();
            m_savedValue = reading;
            LOG_INFO("Game sel changed: " + String(m_savedValue));
        }
    }

    uint8_t read()
    {
        return m_savedValue;
    }
};

/**
 * @brief Game selector that uses number of GPIO to convert them into value
 * that can be used to select game on SBC.
 */
template <int numPins>
class GameSelector
{
    uint8_t m_pins[numPins];

    Button m_applyBtn;

    const bool m_inverse; // Inverse GPIO logic.

    uint8_t m_savedValue = 0;

    /**
     * @brief Read selector's value constructed from binary reads of gamesel pins.
     */
    uint8_t readPins()
    {
        uint8_t res = 0;
        for (uint8_t i = 0; i < numPins; i++)
        {
            uint8_t val = digitalRead(m_pins[i]);
            if (m_inverse)
            {
                val = !val;
            }

            res |= (val << i);
        }
        return res;
    }

public:
    /**
     * @brief Initialize selector hardware.
     *
     * @param inverse Whether digital 0 means that pin is shorted.
     * @param applyPin Pin to apply game selector choice.
     * @param ... numPins of GPIO.
     *
     */
    GameSelector(uint8_t applyPin, bool applyBtnInverse, bool inverse, ...)
        : m_applyBtn(applyPin, applyBtnInverse), m_inverse(inverse)
    {
        va_list argList;

        if (!numPins)
        {
            return;
        }

        va_start(argList, inverse);
        for (uint8_t i = 0; i < numPins; i++)
        {
            m_pins[i] = va_arg(argList, int);
        }
        va_end(argList);
    }

    void init()
    {
        for (uint8_t i = 0; i < numPins; i++)
        {
            pinMode(m_pins[i], INPUT);
        }

        m_savedValue = readPins();

        m_applyBtn.init();
    }

    void update()
    {
        if (m_applyBtn.clicked())
        {
            m_applyBtn.clearState();
            m_savedValue = readPins();
            LOG_INFO("Game sel changed: " + String(m_savedValue));
        }
    }

    uint8_t read()
    {
        return m_savedValue;
    }
};