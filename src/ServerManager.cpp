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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

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

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.begin();
}