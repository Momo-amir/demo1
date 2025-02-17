#include <WiFi.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 23  // Temperature sensor pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Preferences preferences;
AsyncWebServer server(80);

const char* apSSID = "ESP32-Setup";  // AP Mode SSID
const char* apPassword = "12345678"; // AP Mode Password

void startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}


void setupServer() {
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


    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String newSSID = request->getParam("ssid", true)->value();
            String newPassword = request->getParam("password", true)->value();

            preferences.putString("ssid", newSSID);
            preferences.putString("password", newPassword);
            request->send(200, "text/plain", "✅ WiFi credentials saved! Rebooting...");

            delay(5000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "⚠️ Missing parameters");
        }
    });

    server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request) {
        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);
        request->send(200, "text/plain", String(tempC, 2));
    });


    server.on("/get-ip", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (WiFi.status() == WL_CONNECTED) {
            request->send(200, "text/plain", WiFi.localIP().toString());
        } else {
            request->send(503, "text/plain", "⚠️ Not connected to WiFi");
        }
    });
}


void setup() {
    Serial.begin(115200);
    sensors.begin();

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    preferences.begin("wifi", false);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    Serial.println("Stored SSID: " + ssid);
Serial.println("Stored Password: " + password);

    if (ssid != "" && password != "") {
        WiFi.mode(WIFI_STA); // Start as Station mode
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("Connecting to WiFi");
        
        int timeout = 15;  // Increase timeout for better stability
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

    setupServer();
    server.begin();
}



// Serve WiFi Setup Page
void loop() {
    sensors.requestTemperatures();
    Serial.print("Temperature: ");
    Serial.print(sensors.getTempCByIndex(0));
    Serial.println(" °C");
    delay(50000);
}