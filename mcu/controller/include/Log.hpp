#pragma once
#include <Arduino.h>


#define DEBUG 0
#define INFO 1
#define WARN 2
#define ERR 3
#define NONE 255

#ifndef LOG_LEVEL
#define LOG_LEVEL NONE
#endif

#if LOG_LEVEL != NONE
#include <SoftwareSerial.h>
#endif

#if LOG_LEVEL <= DEBUG
void logDebug(String s);
#define LOG_DEBUG(n) logDebug(n)
#else
#define LOG_DEBUG(n) void()
#endif

#if LOG_LEVEL <= INFO
void logInfo(String s);
#define LOG_INFO(n) logInfo(n)
#else
#define LOG_INFO(n) void()
#endif

#if LOG_LEVEL <= WARN
void logWarning(String s);
#define LOG_WARN(n) logWarning(n)
#else
#define LOG_WARN(n) void()
#endif

#if LOG_LEVEL <= ERR
void logError(String s);
#define LOG_ERROR(n) logError(n)
#else
#define LOG_ERROR(n) void()
#endif

namespace logger
{
#if LOG_LEVEL != NONE
    void init(SoftwareSerial &stream);
#endif
}
