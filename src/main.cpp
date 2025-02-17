#include <Arduino.h>
#include <AsyncTCP.h>
#include <OneWire.h>
#include <ESPAsyncWebServer.h>
#include <DallasTemperature.h>
#include <WiFi.h>


// Define the GPIO pin where the DS18B20 is connected
#define ONE_WIRE_BUS 23  

// Setup OneWire and DallasTemperature library
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi credentials
const char* ssid = "KOLLAB-Public";   
const char* password = "Guest123asd";

// Access Point credentials
const char* apSSID = "ESP32-Temp";
const char* apPassword = "12345678";


AsyncWebServer server(80);


String getTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  return String(tempC, 2);  // Convert to string with 2 decimal places
}



void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP32...");
    sensors.begin();

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int timeout = 10;  // 10 seconds timeout
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        Serial.print(".");
        timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Connected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n⚠️ WiFi Failed! Starting AP Mode...");
        WiFi.softAP(apSSID, apPassword);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
    }

        // Serve the main HTML page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/html", "<h1>ESP32 Temperature</h1><p>Current Temperature: <span id='temp'>Loading...</span> °C</p><script>setInterval(()=>fetch('/temp').then(res=>res.text()).then(temp=>document.getElementById('temp').innerText=temp),2000);</script>");
      });
  
      // Serve the temperature as plain text
      server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/plain", getTemperature());
      });
  
      // Start the server
      server.begin();
}


void loop() {
  //Technically not needed as we are serving the temperature on a webpage - loop is only for testing
    sensors.requestTemperatures();  // Request temperature data
    float tempC = sensors.getTempCByIndex(0);  // Get temperature in Celsius

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");

    delay(5000);  // Read every 5 seconds
}