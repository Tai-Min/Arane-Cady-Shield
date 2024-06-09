#include <Arduino.h>
#if LOG_LEVEL != NONE
#include <SoftwareSerial.h>
#endif
#include <Arduino_FreeRTOS.h>
#include "Log.hpp"
#include "AppTask.hpp"
#include "DisplayTask.hpp"
#include "Config.hpp"

namespace
{
#if LOG_LEVEL != NONE
  SoftwareSerial g_logSerial(Config::Main::GPIO::LOG_SERIAL_RX, Config::Main::GPIO::LOG_SERIAL_TX);
#endif

}

void setup()
{
#if LOG_LEVEL != NONE
  g_logSerial.begin(Config::Main::LOG_BAUD);
  logger::init(g_logSerial);
#endif

  xTaskCreate(
      App::task, "app",
      App::getRequiredStack(), NULL,
      1, NULL);

  xTaskCreate(
      Disp::task, "disp",
      Disp::getRequiredStack(), NULL,
      0, NULL);
}

void loop()
{
}