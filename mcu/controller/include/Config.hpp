#pragma once
#include <Arduino.h>

#define GAME_SELECTOR_ENCODER

namespace Config
{
    namespace Main
    {
        constexpr long LOG_BAUD = 9600;

        namespace GPIO
        {
            constexpr uint8_t LOG_SERIAL_RX = 2;
            constexpr uint8_t LOG_SERIAL_TX = 3;
        }
    }

    namespace AppTask
    {
        constexpr unsigned long BOOT_TIMEOUT_DURATION = 120000; // If boot time exceeds this time the app goes into error state.
        constexpr unsigned long SHUTDOWN_DURATION = 30000;      // How long to wait for shutdown to turn off SBC.
    }

    namespace DisplayTask
    {
        constexpr bool INVERT_DET = true;   // If true then if diode on display is lit then digital low is delivered to MCU.
        constexpr bool INVERT_CTRL = false; // If true then digital low state is considered a button press on the display.

        constexpr uint16_t LOOP_DELAY = 100;              // Delay between display task loop iterations.
        constexpr uint16_t DISABLE_CHECK_DURATION = 2000; // For how long to check whether display is ON or OFF.
        constexpr uint16_t STATE_TRANSITION_DELAY = 2000; // How long to wait for display to change state.

        namespace GPIO
        {
            constexpr uint8_t DISP_DET = 7;
            constexpr uint8_t DISP_CTRL = 6;
        }
    }

    namespace Communication
    {
        constexpr uint8_t SBC_HB_MAX_RETRIES = 30;          // How many retries of SBC heartbeat until MCU decides that it's disconnected.
        constexpr unsigned long SBC_HB_CHECK_PERIOD = 2000; // How frequently check for SBC heartbeat.

        constexpr uint8_t DEVICE_ID = 1; // Slave ID

        constexpr unsigned long SERIAL_BAUD = 19200;
        constexpr unsigned long SERIAL_TIMEOUT = 50;

        namespace Coils // Coils (inputs to MCU)
        {

            constexpr uint8_t Q_SHUTDOWN_FLAG_ADDR = 0; // Info from SBC that it's shutting down.
            constexpr uint8_t Q_DISPLAY_STATE_ADDR = 1;
            constexpr uint8_t Q_JOY1_ENA_FLAG_ADDR = 2;
            constexpr uint8_t Q_JOY2_ENA_FLAG_ADDR = 3;
        }

        namespace Inputs // Inputs (outputs from MCU)
        {
            constexpr uint8_t I_SHUTDOWN_REQ_ADDR = 0; // Request from MCU to shutdown SBC.
        }

        namespace HoldingRegs // Holding regs (inputs to MCU)
        {
            constexpr uint8_t AQ_SBC_HB_CNTR_ADDR = 0;
            constexpr uint8_t AQ_JOY1_LED_BRIGHTNESS_ADDR = 1;
            constexpr uint8_t AQ_JOY2_LED_BRIGHTNESS_ADDR = 2;
        }
        namespace InputRegs // Input regs (outputs from MCU)
        {
            constexpr uint8_t AI_MCU_HB_CNTR_ADDR = 0;
            constexpr uint8_t AI_MCU_GAMESEL_ADDR = 1;
        }
    }

    namespace LightEffector
    {
        constexpr float BLINKING_RATIO = 0.1;       // Speed of blinking tied to delta time.
        constexpr float FAST_BLINKING_RATIO = 0.75; // Speed of fast blinking tied to delta time.

        namespace GPIO
        {
            constexpr uint8_t JOY1_LED = 8;
            constexpr uint8_t JOY2_LED = 9;
        }
    }

    namespace GameSelector
    {
        constexpr bool APPLY_BTN_LOGIC_INVERTED = true;

#ifdef GAME_SELECTOR_ENCODER
        constexpr bool ENCODER_DIR_INVERTED = true;
#else
        constexpr bool SELECTORS_LOGIC_INVERTED = true;
#endif

        namespace GPIO
        {
            constexpr uint8_t APPLY_BTN = 12;

            constexpr uint8_t SELECTOR_NUM_PINS = 4; // Edit object declaration with more pins if changed!
            constexpr uint8_t SELECTOR_BITS[SELECTOR_NUM_PINS] = {A3, A2, A1, A0};
        }
    }

    namespace SBC
    {
        constexpr bool LOGIC_INVERTED = true;

        namespace GPIO
        {
            constexpr uint8_t RPI_PWR_CTRL = 5;
        }
    }

    namespace Joy
    {
        constexpr bool LOGIC_INVERTED = true;

        namespace GPIO
        {
            constexpr uint8_t JOY1_CTRL = 10;
            constexpr uint8_t JOY2_CTRL = 11;
        }
    }

    namespace PwrButton
    {
        constexpr unsigned long CLICK_TIMEOUT = 500;
        constexpr unsigned long LONG_PRESS_MIN_TIME = 3000;

        constexpr bool LOGIC_INVERTED = true;

        namespace GPIO
        {
            constexpr uint8_t RPI_PWR_BTN = 4;
        }
    }

    namespace PWM
    {
        constexpr uint8_t NUM_MCU_PINS = 13;
    }
}
