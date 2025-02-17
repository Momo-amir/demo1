#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 23  // Temperature sensor pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Preferences preferences;
AsyncWebServer server(80);

const char* apSSID = "ESP32-Temp";
const char* apPassword = "12345678";

void setup() {
    Serial.begin(115200);
    sensors.begin();
    
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("⚠️ SPIFFS Mount Failed");
        return;
    }

    // Start WiFi connection process
    preferences.begin("wifi", false);
    String ssid = preferences.getString("ssid", "KOLLAB-Public");
    String password = preferences.getString("password", "Guest123asd");

    WiFi.mode(WIFI_AP_STA);  // Ensure both AP & Station mode are active

    if (ssid != "" && password != "") {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("🔄 Connecting to WiFi");
        
        int timeout = 10;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(1000);
            Serial.print(".");
            timeout--;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Connected to WiFi");
        Serial.print("📶 IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n⚠️ WiFi Failed! Starting AP Mode...");
    }

    // Always enable AP mode
    WiFi.softAP(apSSID, apPassword);
    delay(1000);  // Ensure AP initializes properly
    Serial.print("📡 AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Serve files from SPIFFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/script.js", "application/javascript");
    });

    // Endpoint to get temperature
    server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);
        request->send(200, "text/plain", String(tempC, 2));
    });

    // Endpoint to update WiFi credentials
    server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String newSSID = request->getParam("ssid", true)->value();
            String newPassword = request->getParam("password", true)->value();
            preferences.putString("ssid", newSSID);
            preferences.putString("password", newPassword);
            request->send(200, "text/plain", "✅ WiFi credentials saved! Rebooting...");
            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "⚠️ Missing parameters");
        }
    });

    server.begin();
}
void loop() {
    sensors.requestTemperatures();
    Serial.print("Temperature: ");
    Serial.print(sensors.getTempCByIndex(0));
    Serial.println(" °C");
    delay(5000);
}
