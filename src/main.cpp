#include <Arduino.h>
#include "WiFiManager.h"
#include "ServerManager.h"
#include "TemperatureLogger.h"
#include "FactoryReset.h"

void setup() {
    Serial.begin(115200);
    delay(2000);

    // Start WiFi connection
    WiFiManager::initWiFi();

    // Start the web server
    ServerManager::initServer();

    // Start temperature sensor
    TemperatureLogger::init();

}

void loop() {
    checkFactoryReset();
    TemperatureLogger::logAndBroadcast();
    delay(10000);
}