#include "LightEffectors.hpp"
#include "SoftPWM.hpp"
#include "Config.hpp"

LightEffectors::LightEffectors(uint8_t pin1, uint8_t pin2) : m_pin1(pin1), m_pin2(pin2)
{
}

void LightEffectors::init()
{
    pinMode(m_pin1, OUTPUT);
    pinMode(m_pin2, OUTPUT);

    SoftPwmInit();
    SoftPWMAttach(m_pin1);
    SoftPWMAttach(m_pin2);

    SoftPwmWrite(m_pin1, 0);
    SoftPwmWrite(m_pin2, 0);
}

void LightEffectors::update()
{
    if (m_fstScan)
    {
        m_fstScan = false;
        m_effectStamp = millis();
        return;
    }

    unsigned long delta = millis() - m_effectStamp;

    if (delta == 0)
    {
        return;
    }

    switch (m_effect)
    {
    case Effect::OFF:
        SoftPwmWrite(m_pin1, 0);
        SoftPwmWrite(m_pin2, 0);
        break;
    case Effect::BLINKING:
        updatePwm(delta, Config::LightEffector::BLINKING_RATIO);

        SoftPwmWrite(m_pin1, m_pwmEffectVal);
        SoftPwmWrite(m_pin2, m_pwmEffectVal);
        break;
    case Effect::MANUAL:
        SoftPwmWrite(m_pin1, m_pwmVal1);
        SoftPwmWrite(m_pin2, m_pwmVal2);
        break;
    case Effect::ALTERNATING_BLINKING:
        updatePwm(delta, Config::LightEffector::BLINKING_RATIO);

        SoftPwmWrite(m_pin1, m_pwmEffectVal);
        SoftPwmWrite(m_pin2, 255 - m_pwmEffectVal);
        break;
    case Effect::FAST_BLINKING:
        updatePwm(delta, Config::LightEffector::FAST_BLINKING_RATIO);

        SoftPwmWrite(m_pin1, m_pwmEffectVal);
        SoftPwmWrite(m_pin2, m_pwmEffectVal);
        break;
    }

    m_effectStamp = millis();
}

void LightEffectors::updatePwm(unsigned long delta, float ratio)
{
    float pwm = m_pwmEffectVal;

    if (m_inverse)
    {
        pwm -= delta * ratio;

        if (pwm <= 0)
        {
            m_inverse = false;
            pwm = 0;
        }
    }
    else
    {
        pwm += delta * ratio;

        if (pwm >= 255)
        {
            m_inverse = true;
            pwm = 255;
        }
    }

    m_pwmEffectVal = pwm;
}