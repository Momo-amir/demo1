#include "TemperatureLogger.h"
#include "ServerManager.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <time.h>

#define ONE_WIRE_BUS 23
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void TemperatureLogger::init() {
    sensors.begin();
    configTime(0, 0, "pool.ntp.org");  // Sync time from NTP server
    Serial.println("Waiting for NTP time sync...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\n✅ Time synced!");
}

void TemperatureLogger::logAndBroadcast() {
    static unsigned long lastBroadcastTime = 0;
    const unsigned long BROADCAST_INTERVAL = 5 * 60 * 1000;  // 5 minutes

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    // Get formatted time
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

        // Send JSON to WebSocket clients
        String json = "{\"timestamp\":\"" + String(timeStr) + "\",\"temp\":\"" + String(tempC, 2) + "\"}";
        ServerManager::ws.textAll(json);

        Serial.println("📡 Sent WebSocket update: " + json);
    }


}