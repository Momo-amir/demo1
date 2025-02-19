#include "ServerManager.h"
#include <SPIFFS.h>
#include "WiFiManager.h"

AsyncWebServer ServerManager::server(80);
AsyncWebSocket ServerManager::ws("/ws");

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    }
}

void ServerManager::initServer() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Serve static files
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // If the system is in Access Point mode, serve setup.html; otherwise, serve index.html.
        if (WiFi.getMode() == WIFI_AP) {
            request->send(SPIFFS, "/setup.html", "text/html");
        } else {
            request->send(SPIFFS, "/index.html", "text/html");
        }
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "application/javascript");
    });

    server.on("/header.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/header.html", "text/html");
    });
    server.on("/setup.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/setup.html", "text/html");
    });

    // WiFi connect endpoint
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String newSSID = request->getParam("ssid", true)->value();
            String newPassword = request->getParam("password", true)->value();
            
            Preferences preferences;
            preferences.begin("wifi", false);
            preferences.putString("ssid", newSSID);
            preferences.putString("password", newPassword);
            preferences.end();
            
            request->send(200, "text/plain", "✅ WiFi credentials saved! Rebooting...");
            delay(5000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "⚠️ Missing parameters");
        }
    });

    // Return logged temperature data
    server.on("/tempdata", HTTP_GET, [](AsyncWebServerRequest *request) {
        File f = SPIFFS.open("/temp.csv", FILE_READ);
        if (!f) {
            request->send(500, "text/plain", "Failed to read file");
            return;
        }
        String data = "";
        while (f.available()) {
            data += char(f.read());
        }
        f.close();
        request->send(200, "text/plain", data);
    });

    // Return ESP32 IP address
    server.on("/get-ip", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (WiFi.status() == WL_CONNECTED) {
            request->send(200, "text/plain", WiFi.localIP().toString());
        } else {
            request->send(503, "text/plain", "⚠️ Not connected to WiFi");
        }
    });

    // CSV file download
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/temp.csv", "text/csv");
        response->addHeader("Content-Disposition", "attachment; filename=temp.csv");
        request->send(response);
    });
    
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
}
