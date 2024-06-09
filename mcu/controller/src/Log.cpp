#include "Log.hpp"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#if LOG_LEVEL != NONE
namespace
{
    SoftwareSerial *g_stream;
    SemaphoreHandle_t g_logSemapthore = xSemaphoreCreateMutex();
}

namespace logger
{
    void init(SoftwareSerial &stream)
    {
        g_stream = &stream;
    }

}

void logDebug(String s)
{
    if (xSemaphoreTake(g_logSemapthore, (TickType_t)5) == pdTRUE)
    {
        g_stream->print(F("D: "));
        g_stream->println(s);
        xSemaphoreGive(g_logSemapthore);
    }
}

void logInfo(String s)
{
    if (xSemaphoreTake(g_logSemapthore, (TickType_t)5) == pdTRUE)
    {
        g_stream->print(F("I: "));
        g_stream->println(s);
        xSemaphoreGive(g_logSemapthore);
    }
}

void logWarning(String s)
{
    if (xSemaphoreTake(g_logSemapthore, (TickType_t)5) == pdTRUE)
    {
        g_stream->print(F("W: "));
        g_stream->println(s);
        xSemaphoreGive(g_logSemapthore);
    }
}

void logError(String s)
{
    if (xSemaphoreTake(g_logSemapthore, (TickType_t)5) == pdTRUE)
    {
        g_stream->print(F("E: "));
        g_stream->println(s);
        xSemaphoreGive(g_logSemapthore);
    }
}
#endif