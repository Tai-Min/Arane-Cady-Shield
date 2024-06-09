#pragma once
#include <Arduino.h>

/**
 * @brief Initialize hardware for soft PWM.
*/
void SoftPwmInit();

/**
 * @brief Attach pin to soft PWM.
*/
void SoftPWMAttach(uint8_t pin);

/**
 * @brief Detach pin from soft PWM.
*/
void SoftPWMDetach(uint8_t pin);

/**
 * @brief Set new pwm value to given pin.
*/
void SoftPwmWrite(uint8_t pin, uint8_t val);