#include "TemperatureLogger.h"
#include "ServerManager.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <time.h>

// Setup your OneWire and DallasTemperature objects
#define ONE_WIRE_BUS 23
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// File-scope ticker and callback function
Ticker temperatureTicker;

void broadcastTemperature() {
    TemperatureLogger::logAndBroadcast();
}

namespace TemperatureLogger {

void init() {

    sensors.begin();

    setenv("TZ", "BST0BST,M3.5.0/1,M10.5.0", 1); 
    tzset();
    configTime(3600, 0, "pool.ntp.org");  // Sync time from NTP server - 1 hour offset (3600 seconds) should be dynamic based on timezone   
    Serial.println("Waiting for NTP time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\n✅ Time synced!");
}

void logAndBroadcast() {
    static unsigned long lastBroadcastTime = 0;
    const unsigned long BROADCAST_INTERVAL = 5 * 60 * 1000;  // 5 minutes

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("⚠️ Failed to get time");
        return;
    }

    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Log to CSV
    File logFile = SPIFFS.open("/temp.csv", FILE_APPEND);
    if (logFile) {
        logFile.printf("%s,%.2f\n", timeStr, tempC);
        logFile.close();
    } else {
        Serial.println("⚠️ Failed to open /temp.csv for writing");
    }

    // Only send WebSocket updates every 5 minutes
    if (millis() - lastBroadcastTime >= BROADCAST_INTERVAL) {
        lastBroadcastTime = millis();
        String json = "{\"timestamp\":\"" + String(timeStr) + "\",\"temp\":\"" + String(tempC, 2) + "\"}";
        ServerManager::ws.textAll(json);
        Serial.println("📡 Sent WebSocket update: " + json);
    }
}

void startScheduler() {
    // Schedule the broadcast every 300 seconds (5 minutes)
    logAndBroadcast();

    temperatureTicker.attach(300, broadcastTemperature);
}

} // namespace TemperatureLogger