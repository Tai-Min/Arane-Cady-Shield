#include "Comm.hpp"
#include <Arduino_FreeRTOS.h>
#include <MSlave.h>
#include "Log.hpp"
#include "State.hpp"

#include "Config.hpp"

namespace
{
    bool g_connected = false;
    uint8_t g_sbcHeartbeatRetries = 0;

    MSlave<Config::Communication::Coils::Q_JOY2_ENA_FLAG_ADDR + 1,
           Config::Communication::Inputs::I_SHUTDOWN_REQ_ADDR + 1,
           Config::Communication::HoldingRegs::AQ_JOY2_LED_BRIGHTNESS_ADDR + 1,
           Config::Communication::InputRegs::AI_MCU_GAMESEL_ADDR + 1>
        g_server;
}

namespace Comm
{
    void init();
    void update();
    bool connected();

    /**
     * @brief Update State.hpp variables with data from MODBUS server.
     */
    void updateState();

    /**
     * @brief Check SBC heartbeat status.
     */
    void sbcHeartbeatCheck();

    /**
     * @brief Perform MCU heartbeat.
     */
    void doMcuHeartbeat();

    void init()
    {
        Serial.begin(Config::Communication::SERIAL_BAUD);
        Serial.setTimeout(Config::Communication::SERIAL_TIMEOUT);
        g_server.begin(Config::Communication::DEVICE_ID, Serial);

        LOG_INFO(F("Comm start"));
    }

    void update()
    {
        if (g_server.available())
        {
            g_server.read();
        }

        updateState();
        sbcHeartbeatCheck();
        doMcuHeartbeat();
    }

    bool connected()
    {
        return g_connected;
    }

    void updateState()
    {
        State::setShutdownFlag(g_server.digitalRead(COIL, Config::Communication::Coils::Q_SHUTDOWN_FLAG_ADDR));
        State::setDisplayState(g_server.digitalRead(COIL, Config::Communication::Coils::Q_DISPLAY_STATE_ADDR));
        State::setJoy1Enable(g_server.digitalRead(COIL, Config::Communication::Coils::Q_JOY1_ENA_FLAG_ADDR));
        State::setJoy2Enable(g_server.digitalRead(COIL, Config::Communication::Coils::Q_JOY2_ENA_FLAG_ADDR));

        g_server.digitalWrite(INPUT, Config::Communication::Inputs::I_SHUTDOWN_REQ_ADDR, State::getShutdownRequest());

        State::setJoy1Brightness(g_server.analogRead(HOLDING_REG, Config::Communication::HoldingRegs::AQ_JOY1_LED_BRIGHTNESS_ADDR));
        State::setJoy2Brightness(g_server.analogRead(HOLDING_REG, Config::Communication::HoldingRegs::AQ_JOY2_LED_BRIGHTNESS_ADDR));

        g_server.analogWrite(INPUT_REG, Config::Communication::InputRegs::AI_MCU_GAMESEL_ADDR, State::getSelectorValue());
    }

    void sbcHeartbeatCheck()
    {
        static unsigned long g_sbcHeartbeatCheckStamp = millis();
        static uint16_t sbcHeartbeatCntr = 0;

        // Not enough time passed to perform next heartbead check
        if (millis() - g_sbcHeartbeatCheckStamp < Config::Communication::SBC_HB_CHECK_PERIOD)
        {
            return;
        }

        g_sbcHeartbeatCheckStamp = millis();

        LOG_DEBUG(F("SBC HB"));

        uint16_t val = g_server.analogRead(HOLDING_REG, Config::Communication::HoldingRegs::AQ_SBC_HB_CNTR_ADDR);

        // Value didn't change in SBC, consider as no heartbeat
        if (val == sbcHeartbeatCntr)
        {
            if (g_connected)
            {
                g_sbcHeartbeatRetries++;
            }

            if (g_sbcHeartbeatRetries >= Config::Communication::SBC_HB_MAX_RETRIES)
            {
                LOG_DEBUG(F("SBC nok"));

                g_connected = false;
                g_sbcHeartbeatRetries = Config::Communication::SBC_HB_MAX_RETRIES;
            }
        }
        else
        {
            LOG_DEBUG(F("SBC ok"));

            g_sbcHeartbeatRetries = 0;
            g_connected = true;
            sbcHeartbeatCntr = val;
        }
    }

    void doMcuHeartbeat()
    {
        static uint16_t g_mcuHeartbeatCntr = 0;
        g_mcuHeartbeatCntr++;
        g_server.analogWrite(INPUT_REG, Config::Communication::InputRegs::AI_MCU_HB_CNTR_ADDR, g_mcuHeartbeatCntr);
    }
}