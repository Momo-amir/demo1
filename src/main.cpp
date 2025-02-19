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
AsyncWebSocket ws("/ws"); // WebSocket on "/ws" endpoint

const char* apSSID = "ESP32-Setup";  // AP Mode SSID
const char* apPassword = "12345678"; // AP Mode Password

// WebSocket event handler (if needed for client messages)
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    }
}

void startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void setupServer() {
    // Serve static files from SPIFFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/script.js", "application/javascript");
    });

    // WiFi connect endpoint
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
  
    // Endpoint to return the logged temperature data
    server.on("/tempdata", HTTP_GET, [](AsyncWebServerRequest *request) {
        File f = SPIFFS.open("/temp.csv", FILE_READ);
        if (!f) {
            request->send(500, "text/plain", "Failed to read file");
            return;
        }
        String data = "";
        while(f.available()){
            data += char(f.read());
        }
        f.close();
        request->send(200, "text/plain", data);
    });

    // Endpoint to return the ESP32 IP address.
    server.on("/get-ip", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (WiFi.status() == WL_CONNECTED) {
            request->send(200, "text/plain", WiFi.localIP().toString());
        } else {
            request->send(503, "text/plain", "⚠️ Not connected to WiFi");
        }
    });



    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        File f = SPIFFS.open("/temp.csv", FILE_READ);
        if (!f) {
            request->send(500, "text/plain", "Failed to open CSV file");
            return;
        }
        String data = "";
        while (f.available()){
            data += char(f.read());
        }
        f.close();
    
        AsyncWebServerResponse *response = request->beginResponse(200, "text/csv", data);
        response->addHeader("Content-Disposition", "attachment; filename=temp.csv");
        request->send(response);
    });
    // Add the websocket handler
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
}

void logAndBroadcastTemperature() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
  
    // Append temperature reading to '/temp.csv'
    File logFile = SPIFFS.open("/temp.csv", FILE_APPEND);
    if (logFile) {
        String dataLine = String(millis()) + "," + String(tempC, 2) + "\n";
        logFile.print(dataLine);
        logFile.close();
    } else {
        Serial.println("Failed to open /temp.csv for appending");
    }
  
    // Prepare a JSON message with timestamp and temperature.
    String json = "{\"timestamp\":\"" + String(millis()) + "\",\"temp\":\"" + String(tempC, 2) + "\"}";
  
    // Broadcast the temperature reading to all connected websocket clients.
    ws.textAll(json);
  
    // Also print temperature to serial monitor
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");
}

void setup() {
    Serial.begin(115200);
    delay(5000);
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
    
    Serial.println("Server starting up with satuts: " + WiFi.status()); 
    setupServer();
    server.begin();
}

// In this version, instead of an HTTP endpoint to get a temperature reading,
// we log and then broadcast via websocket periodically.
void loop() {
    logAndBroadcastTemperature();
    Serial.println(WiFi.localIP());

    // delay here defines the interval between readings (e.g. 50 sec)
    delay(20000);
}