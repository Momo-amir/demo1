#include "FactoryReset.h"
#include "WiFiManager.h"  // Include WiFiManager to access preferences
#include <WiFi.h>
#include <SPIFFS.h>

#define RESET_BUTTON_PIN 5
#define LED_PIN 2
#define RESET_HOLD_TIME 20000

void factoryReset() {
    Serial.println("\n🔄 Factory Reset Started...");

    // Clear WiFi credentials from Preferences
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    Serial.println("✅ WiFi Credentials Cleared");

    // Disconnect WiFi
    WiFi.disconnect(true, true);
    Serial.println("✅ WiFi Disconnected");

    // Delete temperature log file
    if (SPIFFS.exists("/temp.csv")) {
        SPIFFS.remove("/temp.csv");
        Serial.println("✅ temp.csv Deleted");
    } else {
        Serial.println("⚠️ temp.csv Not Found");
    }

    // Blink LED 3 times
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        digitalWrite(LED_PIN, LOW);
        delay(300);
    }

    Serial.println("🚀 Restarting ESP32...");
    delay(500);
    ESP.restart();
}

void checkFactoryReset() {
    static unsigned long buttonPressTime = 0;

    if (digitalRead(RESET_BUTTON_PIN) == HIGH) {
        if (buttonPressTime == 0) {
            buttonPressTime = millis();
            Serial.println("🟠 Hold button for 20s to reset...");
        }

        if (millis() - buttonPressTime >= RESET_HOLD_TIME) {
            factoryReset();
        }
    } else {
        buttonPressTime = 0;  // Reset timer if button is released
    }
}