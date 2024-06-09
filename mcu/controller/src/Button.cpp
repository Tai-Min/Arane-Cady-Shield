#include "Button.hpp"
#include "Config.hpp"

Button::Button(uint8_t pin, bool activeLow) : m_pin(pin)
{
    if (activeLow)
    {
        RELEASED = true;
        PRESSED = false;
    }
}

void Button::init()
{
    pinMode(m_pin, INPUT);
}

void Button::update()
{
    if(m_fstScan)
    {
        m_fstScan = false;
        m_previousState = digitalRead(m_pin);;
        return;
    }

    bool state = digitalRead(m_pin);

    if (state != m_previousState)
    {
        if (state == PRESSED)
        {
            m_pressedStartStamp = millis();
        }
        else
        {
            if (millis() - m_pressedStartStamp <= Config::PwrButton::CLICK_TIMEOUT)
            {
                m_clicked = true;
            }
        }
        m_previousState = state;
    }
    else if (state == PRESSED)
    {
        if (millis() - m_pressedStartStamp >= Config::PwrButton::LONG_PRESS_MIN_TIME)
        {
            m_longPressed = true;
        }
    }
}

bool Button::pressed()
{
    bool state = digitalRead(m_pin);
    return state == PRESSED;
}

bool Button::longPressed()
{
    return m_longPressed;
}

bool Button::clicked()
{
    return m_clicked;
}

void Button::clearState()
{
    m_clicked = false;
    m_longPressed = false;
}