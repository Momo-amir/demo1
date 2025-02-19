#include "WiFiManager.h"

Preferences preferences;
const char* apSSID = "ESP32-Setup";
const char* apPassword = "12345678";

void WiFiManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiManager::initWiFi() {
    preferences.begin("wifi", false);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");

    if (ssid != "" && password != "") {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("Connecting to WiFi");

        int timeout = 15;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(1000);
            Serial.print(".");
            timeout--;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Connected to WiFi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\n⚠️ WiFi Failed! Switching to AP Mode...");
            startAPMode();
        }
    } else {
        Serial.println("\n⚠️ No WiFi credentials found. Starting AP Mode...");
        startAPMode();
    }
}