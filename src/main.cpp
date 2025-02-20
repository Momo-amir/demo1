#include <Arduino.h>
#include "WiFiManager.h"
#include "ServerManager.h"
#include "TemperatureLogger.h"
#include "FactoryReset.h"




// Structed main based on C# progam.cs or startup.cs philosophy
// as it is the main entry point for the program and should not hold actual logic
void setup() {
    Serial.begin(115200); 
    delay(2000);
    pinMode(LED, OUTPUT);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    // Start WiFi connection
    WiFiManager::initWiFi();

    // Start the web server
    ServerManager::initServer();

    // Start temperature sensor
    TemperatureLogger::init();
    TemperatureLogger::startScheduler();



}

void loop() {

    checkFactoryReset();
    delay(1000);
};