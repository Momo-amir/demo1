#ifndef TEMPERATURELOGGER_H
#define TEMPERATURELOGGER_H

#include <DallasTemperature.h>
#include <OneWire.h>
#include <Ticker.h>

namespace TemperatureLogger {
    void init();
    void logAndBroadcast();
    void startScheduler();
}

#endif // TEMPERATURELOGGER_H