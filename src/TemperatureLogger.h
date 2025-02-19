#ifndef TEMPERATURELOGGER_H
#define TEMPERATURELOGGER_H

#include <DallasTemperature.h>
#include <OneWire.h>

namespace TemperatureLogger {
    void init();
    void logAndBroadcast();
}

#endif // TEMPERATURELOGGER_H