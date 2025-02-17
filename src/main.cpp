#include <Arduino.h>
#include <AsyncTCP.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Define the GPIO pin where the DS18B20 is connected
#define ONE_WIRE_BUS 23  // Change if using a different GPIO

// Setup OneWire and DallasTemperature library
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
  
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount());
  Serial.println(" DS18B20 sensor(s)");
}


void loop() {
    sensors.requestTemperatures();  // Request temperature data
    float tempC = sensors.getTempCByIndex(0);  // Get temperature in Celsius

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");

    delay(2000);  // Read every 2 seconds
}