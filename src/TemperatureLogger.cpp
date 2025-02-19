#include "TemperatureLogger.h"
#include "ServerManager.h"
#include <SPIFFS.h>

#define ONE_WIRE_BUS 23
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void TemperatureLogger::init() {
    sensors.begin();
}

void TemperatureLogger::logAndBroadcast() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    File logFile = SPIFFS.open("/temp.csv", FILE_APPEND);
    if (logFile) {
        String dataLine = String(millis()) + "," + String(tempC, 2) + "\n";
        logFile.print(dataLine);
        logFile.close();
    } else {
        Serial.println("Failed to open /temp.csv for appending");
    }

    String json = "{\"timestamp\":\"" + String(millis()) + "\",\"temp\":\"" + String(tempC, 2) + "\"}";
    ServerManager::ws.textAll(json);

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");
}