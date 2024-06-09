#include "State.hpp"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace
{
    bool g_shutdownReq = false; // from MCU to sbc
    bool g_shutdownFlag = false;
    bool g_displayState = false;
    bool g_joy1Enable = false;
    bool g_joy2Enable = false;
    uint8_t g_joy1Brightness = 0;
    uint8_t g_joy2Brightness = 0;
    uint8_t g_selectorValue = 0;

    SemaphoreHandle_t g_paramSemaphore = xSemaphoreCreateMutex();
}

namespace State
{
    bool getShutdownRequest()
    {
        bool res = false;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_shutdownReq;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setShutdownRequest(bool req)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_shutdownReq = req;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    bool getShutdownFlag()
    {
        bool res = false;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_shutdownFlag;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setShutdownFlag(bool flag)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_shutdownFlag = flag;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    bool getDisplayState()
    {
        bool res = false;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_displayState;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setDisplayState(bool state)
    {
        state = true;
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_displayState = state;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    bool getJoy1Enable()
    {
        bool res;
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_joy1Enable;
            xSemaphoreGive(g_paramSemaphore);
        }
        return res;
    }

    void setJoy1Enable(bool ena)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_joy1Enable = ena;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    bool getJoy2Enable()
    {
        bool res;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_joy2Enable;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setJoy2Enable(bool ena)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_joy2Enable = ena;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    uint8_t getJoy1Brightness()
    {
        uint8_t res;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_joy1Brightness;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setJoy1Brightness(uint8_t val)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_joy1Brightness = val;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    uint8_t getJoy2Brightness()
    {
        uint8_t res;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_joy2Brightness;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setJoy2Brightness(uint8_t val)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_joy2Brightness = val;
            xSemaphoreGive(g_paramSemaphore);
        }
    }

    uint8_t getSelectorValue()
    {
        uint8_t res;

        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            res = g_selectorValue;
            xSemaphoreGive(g_paramSemaphore);
        }

        return res;
    }

    void setSelectorValue(uint8_t val)
    {
        if (xSemaphoreTake(g_paramSemaphore, (TickType_t)5) == pdTRUE)
        {
            g_selectorValue = val;
            xSemaphoreGive(g_paramSemaphore);
        }
    }
}