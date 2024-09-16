#include "AppTask.hpp"
#include <Arduino_FreeRTOS.h>
#include <PinChangeInterrupt.h>
#include "Button.hpp"
#include "SBC.hpp"
#include "USB.hpp"
#include "State.hpp"
#include "LightEffectors.hpp"
#include "GameSelector.hpp"
#include "Log.hpp"
#include "Comm.hpp"
#include "Config.hpp"
#include "DisplayTask.hpp"

namespace
{
    Button g_pwrBtn(Config::PwrButton::GPIO::RPI_PWR_BTN, Config::PwrButton::LOGIC_INVERTED);
    SBC g_sbc(Config::SBC::GPIO::RPI_PWR_CTRL, Config::SBC::LOGIC_INVERTED);

    USB g_joy1(Config::Joy::GPIO::JOY1_CTRL, Config::Joy::LOGIC_INVERTED);
    USB g_joy2(Config::Joy::GPIO::JOY2_CTRL, Config::Joy::LOGIC_INVERTED);

    LightEffectors g_ledCtrl(Config::LightEffector::GPIO::JOY1_LED,
                             Config::LightEffector::GPIO::JOY2_LED);

#ifdef GAME_SELECTOR_ENCODER
    GameSelectorEncoderDisplay g_gameSelector(
        Config::GameSelector::GPIO::APPLY_BTN,
        Config::GameSelector::APPLY_BTN_LOGIC_INVERTED,
        Config::GameSelector::GPIO::SELECTOR_BITS[0],
        Config::GameSelector::GPIO::SELECTOR_BITS[1],
        Config::GameSelector::GPIO::SELECTOR_BITS[2],
        Config::GameSelector::GPIO::SELECTOR_BITS[3],
        Config::GameSelector::GPIO::SELECTOR_NUM_PINS *Config::GameSelector::GPIO::SELECTOR_NUM_PINS);
#else
    GameSelector<Config::GameSelector::GPIO::SELECTOR_NUM_PINS> g_gameSelector(
        Config::GameSelector::GPIO::APPLY_BTN,
        Config::GameSelector::APPLY_BTN_LOGIC_INVERTED,
        Config::GameSelector::SELECTORS_LOGIC_INVERTED,
        Config::GameSelector::GPIO::SELECTOR_BITS[0],
        Config::GameSelector::GPIO::SELECTOR_BITS[1],
        Config::GameSelector::GPIO::SELECTOR_BITS[2],
        Config::GameSelector::GPIO::SELECTOR_BITS[3]);
#endif

    enum class AppState
    {
        OFF,
        BOOTING,
        CONNECTED,
        SHUTTING_DOWN,
        ERROR
    } g_state;

    unsigned long g_bootStamp;
}

namespace App
{
    void task(void *pvParameters __attribute__((unused)));

    void setAppState(AppState s);

    void handleForceShutdown();
    void handleOffState();
    void handleBootState();
    void handleConnectedState();
    void handleShutdownState();
    void handleErrorState();

    // Only for encoder based selector
    void selectorClkISRhandler();

    void logHighwater();

    void task(void *pvParameters __attribute__((unused)))
    {
        bool forceShutdown = false;

        g_sbc.init();
        g_pwrBtn.init();
        g_joy1.init();
        g_joy2.init();
        g_ledCtrl.init();
        g_gameSelector.init();

#ifdef GAME_SELECTOR_ENCODER
        attachPinChangeInterrupt(
            digitalPinToPCINT(
                Config::GameSelector::GPIO::SELECTOR_BITS[0]),
            selectorClkISRhandler, CHANGE);
#endif

        Comm::init();

        setAppState(AppState::OFF);

        while (true)
        {
            Comm::update();
            g_pwrBtn.update();
            g_ledCtrl.update();
            g_gameSelector.update();

            State::setSelectorValue(g_gameSelector.read());

            if (g_pwrBtn.longPressed() && !forceShutdown)
            {
                g_pwrBtn.clearState();
                forceShutdown = true;
                handleForceShutdown();
            }
            else if (!g_pwrBtn.longPressed() && forceShutdown)
            {
                forceShutdown = false;
            }

            g_ledCtrl.setManualBrightness(State::getJoy1Brightness(), State::getJoy2Brightness());

            switch (g_state)
            {
            case AppState::OFF:
                handleOffState();
                break;
            case AppState::BOOTING:
                handleBootState();
                break;
            case AppState::CONNECTED:
                handleConnectedState();
                break;
            case AppState::SHUTTING_DOWN:
                handleShutdownState();
                break;
            case AppState::ERROR:
                handleErrorState();
                break;
            }

            g_pwrBtn.clearState();

            // logHighwater();

            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    void setAppState(AppState s)
    {
        g_state = s;
        switch (s)
        {
        case AppState::OFF:
            g_sbc.off();
            g_joy1.off();
            g_joy2.off();
            g_ledCtrl.setEffect(LightEffectors::Effect::OFF);

            Disp::forceOff();

            State::setShutdownFlag(false); // Just in case that SBC manages
                                           // to write this flag again on shutdown
            State::setShutdownRequest(false);
            break;
        case AppState::BOOTING:
            g_sbc.on();
            g_joy1.off();
            g_joy2.off();
            g_ledCtrl.setEffect(LightEffectors::Effect::BLINKING);

            Disp::forceOff();

            g_bootStamp = millis();
            break;
        case AppState::CONNECTED:
            g_ledCtrl.setEffect(LightEffectors::Effect::MANUAL);
            
            if(State::getJoy1Enable())
            {
                g_joy1.on();
            }
            else
            {
                g_joy1.off();
            }

            if(State::getJoy2Enable())
            {
                g_joy2.on();
            }
            else
            {
                g_joy2.off();
            }

            Disp::removeForceOff();
            break;
        case AppState::SHUTTING_DOWN:
            g_joy1.off();
            g_joy2.off();
            g_ledCtrl.setEffect(LightEffectors::Effect::ALTERNATING_BLINKING);

            Disp::forceOff();

            State::setShutdownFlag(false);
            State::setShutdownRequest(true);

            g_bootStamp = millis();

            break;
        case AppState::ERROR:
            g_ledCtrl.setEffect(LightEffectors::Effect::FAST_BLINKING);
            Disp::removeForceOff();
            State::setDisplayState(true); // Force ON to see error cause

            State::setShutdownFlag(false);
            State::setShutdownRequest(false);

            g_joy1.off();
            g_joy2.off();
            break;
        }
    }

    void handleForceShutdown()
    {
        setAppState(AppState::OFF);

        LOG_INFO(F("Force sdown"));
    }

    void handleOffState()
    {
        g_sbc.off();
        if (g_pwrBtn.clicked())
        {
            setAppState(AppState::BOOTING);

            LOG_INFO(F("Booting"));
        }
    }

    void handleBootState()
    {
        if (Comm::connected())
        {
            setAppState(AppState::CONNECTED);

            LOG_INFO(F("Boot ok"));
        }

        if (millis() - g_bootStamp >= Config::AppTask::BOOT_TIMEOUT_DURATION)
        {
            setAppState(AppState::ERROR);
            LOG_ERROR(F("Boot tout"));
        }
    }

    void handleConnectedState()
    {
        if (!Comm::connected())
        {
            setAppState(AppState::ERROR);

            LOG_ERROR(F("Disconn"));
        }
        if (State::getShutdownFlag())
        {
            setAppState(AppState::SHUTTING_DOWN);

            LOG_INFO(F("SBC sdown"));
        }
        if (g_pwrBtn.clicked())
        {
            setAppState(AppState::SHUTTING_DOWN);

            LOG_INFO(F("Btn sdown"));
        }
    }

    void handleShutdownState()
    {
        if (millis() - g_bootStamp >= Config::AppTask::SHUTDOWN_DURATION)
        {
            setAppState(AppState::OFF);

            LOG_INFO(F("sdown ok"));
        }
    }

    void handleErrorState()
    {
        if (g_pwrBtn.clicked())
        {
            setAppState(AppState::OFF);

            LOG_INFO(F("Btn sdown"));
        }
    }

#ifdef GAME_SELECTOR_ENCODER
    void selectorClkISRhandler()
    {
        g_gameSelector.encoderUpdate();
    }

#endif

    void logHighwater()
    {
        static unsigned long stamp = millis();
        constexpr unsigned long period = 5000;

        if (millis() - stamp >= period)
        {
            uint16_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            LOG_INFO(String(F("app: ")) + String(uxHighWaterMark));
            stamp = millis();
        }
    }
}
