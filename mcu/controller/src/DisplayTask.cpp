#include "DisplayTask.hpp"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "Log.hpp"
#include "State.hpp"
#include "Config.hpp"

namespace
{
    bool ON = true;   // Used to check factual display state.
    bool OFF = false; // Used to check factual display state.

    bool RELEASED = false; // Used to simulate button press on display.
    bool PRESSED = true;   // Used to simulate button press on display.

    bool m_focedOff = false;
    SemaphoreHandle_t g_forceOffSemaphore = xSemaphoreCreateMutex();
}

namespace Disp
{
    void task(void *pvParameters __attribute__((unused)));
    void forceOff();
    void removeForceOff();

    /**
     * @brief Simulate button click with delay.
     */
    void click();

    /**
     * @brief Log stack usage periodically.
     */
    void logHighwater();

    void task(void *pvParameters __attribute__((unused)))
    {
        if (Config::DisplayTask::INVERT_DET)
        {
            ON = false;
            OFF = true;
        }
        if (Config::DisplayTask::INVERT_CTRL)
        {
            PRESSED = false;
            RELEASED = true;
        }

        pinMode(Config::DisplayTask::GPIO::DISP_CTRL, OUTPUT);
        pinMode(Config::DisplayTask::GPIO::DISP_DET, INPUT);

        digitalWrite(Config::DisplayTask::GPIO::DISP_CTRL, RELEASED);

        while (true)
        {
            bool forcedOff = false;
            if (xSemaphoreTake(g_forceOffSemaphore, (TickType_t)5) == pdTRUE)
            {
                forcedOff = m_focedOff;
                xSemaphoreGive(g_forceOffSemaphore);
            }

            if (State::getDisplayState() && !forcedOff)
            {
                if (digitalRead(Config::DisplayTask::GPIO::DISP_DET) == OFF)
                {
                    click();

                    LOG_ERROR(F("Disp on"));

                    vTaskDelay(pdMS_TO_TICKS(Config::DisplayTask::STATE_TRANSITION_DELAY));
                }
            }
            else
            {
                bool isOff = false;
                for (uint8_t i = 0; i < Config::DisplayTask::DISABLE_CHECK_DURATION / Config::DisplayTask::LOOP_DELAY; i++)
                {
                    if (digitalRead(Config::DisplayTask::GPIO::DISP_DET) == OFF)
                    {
                        isOff = true;
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(Config::DisplayTask::LOOP_DELAY));
                }

                if (!isOff)
                {
                    click();

                    LOG_ERROR(F("Disp off"));

                    vTaskDelay(pdMS_TO_TICKS(Config::DisplayTask::STATE_TRANSITION_DELAY));
                }
            }

            // logHighwater();

            vTaskDelay(pdMS_TO_TICKS(Config::DisplayTask::LOOP_DELAY));
        }
    }

    void forceOff()
    {
        if (xSemaphoreTake(g_forceOffSemaphore, (TickType_t)5) == pdTRUE)
        {
            m_focedOff = true;
            xSemaphoreGive(g_forceOffSemaphore);
        }
    }

    void removeForceOff()
    {
        if (xSemaphoreTake(g_forceOffSemaphore, (TickType_t)5) == pdTRUE)
        {
            m_focedOff = false;
            xSemaphoreGive(g_forceOffSemaphore);
        }
    }

    void click()
    {
        digitalWrite(Config::DisplayTask::GPIO::DISP_CTRL, PRESSED);
        vTaskDelay(pdMS_TO_TICKS(400));
        digitalWrite(Config::DisplayTask::GPIO::DISP_CTRL, RELEASED);
    }

    void logHighwater()
    {
        static unsigned long stamp = millis();
        constexpr unsigned long period = 5000;

        if (millis() - stamp >= period)
        {
            uint16_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            LOG_INFO(String(F("disp: ")) + String(uxHighWaterMark));
            stamp = millis();
        }
    }
}