#pragma once

class Encoder
{
private:
    bool BACKWARD = false;
    bool FORWARD = true;

    volatile uint8_t m_value = 0;
    bool m_isDebouncing = false;

    const uint8_t m_clkPin;
    const uint8_t m_dirPin;
    const uint8_t m_maxValue;

    const unsigned long m_debounceTime = 10;
    unsigned long m_debounceStamp = 0;
    ;

public:
    Encoder(uint8_t clkPin, uint8_t dirPin, bool dirInverted, uint8_t maxValue)
        : m_clkPin(clkPin), m_dirPin(dirPin), m_maxValue(maxValue)
    {
        if(dirInverted)
        {
            BACKWARD = true;
            FORWARD = false;
        }
    }

    void init()
    {
        pinMode(m_clkPin, INPUT);
        pinMode(m_dirPin, INPUT);
    }

    void update()
    {
        unsigned long currStamp = millis();

        if (m_isDebouncing)
        {
            if (currStamp - m_debounceStamp >= m_debounceTime)
            {
                m_isDebouncing = false;
            }
            else
            {
                return;
            }
        }

        if (!digitalRead(m_clkPin))
        {
            m_isDebouncing = true;
            m_debounceStamp = currStamp;

            if (digitalRead(m_dirPin) == FORWARD)
            {
                m_value++;
                if (m_value > m_maxValue)
                {
                    m_value = 0;
                }
            }
            else
            {
                m_value--;
                if (m_value > m_maxValue)
                {
                    m_value = m_maxValue;
                }
            }
        }
    }

    uint8_t read()
    {
        return m_value;
    }
};