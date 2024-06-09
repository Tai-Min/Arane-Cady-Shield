#include "SoftPWM.hpp"
#include "Config.hpp"

namespace
{
    constexpr uint8_t FREE_PIN = 255;

    volatile uint8_t g_numUsedPins = 0;
    volatile uint8_t g_usedPins[Config::PWM::NUM_MCU_PINS];
    volatile uint8_t g_vals[Config::PWM::NUM_MCU_PINS];
}

ISR(TIMER1_COMPA_vect)
{
    static uint16_t cntr = 0;
    TCNT1 = 0;

    cntr++;
    if (cntr == 256)
    {
        for (uint8_t i = 0; i < g_numUsedPins; i++)
        {
            if (g_vals[i] != 0)
            {
                uint8_t bit = digitalPinToBitMask(g_usedPins[i]);
                uint8_t port = digitalPinToPort(g_usedPins[i]);
                volatile uint8_t *out = portOutputRegister(port);

                *out |= bit;
            }
        }
        cntr = 0;
    }

    for (uint8_t i = 0; i < g_numUsedPins; i++)
    {
        if (cntr == g_vals[i])
        {
            uint8_t bit = digitalPinToBitMask(g_usedPins[i]);
            uint8_t port = digitalPinToPort(g_usedPins[i]);
            volatile uint8_t *out = portOutputRegister(port);

            *out &= ~bit;
        }
    }
}

void SoftPwmInit()
{
    for (uint8_t i = 0; i < Config::PWM::NUM_MCU_PINS; i++)
    {
        g_usedPins[i] = FREE_PIN;
        g_vals[i] = 0;
    }

    cli();
    // Reset timer from Arduino stuff
    TCCR1A = 0;
    TCNT1 = 0;

    // 64 prescaler
    TCCR1A |= (1 << CS11) | (1 << CS10);

    // 2 ticks per ISR
    OCR1A = 2;

    TIMSK1 |= (1 << OCIE1A);
    sei();
}

void SoftPWMAttach(uint8_t pin)
{
    if (pin >= Config::PWM::NUM_MCU_PINS)
    {
        return;
    }

    for (uint8_t i = 0; i < Config::PWM::NUM_MCU_PINS; i++)
    {
        if (g_usedPins[i] == pin)
        {
            return;
        }
    }

    for (uint8_t i = 0; i < Config::PWM::NUM_MCU_PINS; i++)
    {
        if (g_usedPins[i] == FREE_PIN)
        {

            g_usedPins[i] = pin;
            g_vals[i] = 0;
            g_numUsedPins++;
            break;
        }
    }

    pinMode(pin, OUTPUT);
}

void SoftPWMDetach(uint8_t pin)
{
    if (pin >= Config::PWM::NUM_MCU_PINS)
    {
        return;
    }

    for (uint8_t i = 0; i < Config::PWM::NUM_MCU_PINS; i++)
    {
        if (g_usedPins[i] == pin)
        {
            g_usedPins[i] = FREE_PIN;
            g_numUsedPins--;
            break;
        }
    }

    digitalWrite(pin, LOW);
}

void SoftPwmWrite(uint8_t pin, uint8_t val)
{
    for (uint8_t i = 0; i < Config::PWM::NUM_MCU_PINS; i++)
    {
        if (g_usedPins[i] == pin)
        {
            g_vals[i] = val;
            break;
        }
    }
}