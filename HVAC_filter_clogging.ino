#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

//Modbus TCP server
ModbusIP mb;

//Register addresses
const uint16_t REG_TEMPERATURE = 100;
const uint16_t REG_HUMIDITY    = 101;

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize Modbus TCP server
  mb.server(); //Start Modbus TCP server on port 502 (default)
  mb.addHreg(REG_TEMPERATURE, 0);
  mb.addHreg(REG_HUMIDITY, 0);

}

void loop() {
  static uint32_t timer = millis();

  //Simulate values of sensor
  if (millis() - timer > 2000) {
    timer = millis();

    float temperature = 22.5 + (rand() % 100) / 10.0;
    float humidity = 40.0 + (rand() % 100) / 10.0;
    mb.Hreg(REG_TEMPERATURE, (uint16_t)(temperature * 10));
    mb.Hreg(REG_HUMIDITY, (uint16_t)(humidity * 10));

    Serial.printf("Updated: Temp = %.1f°C, Hum = %.1f%%\n", temperature, humidity);
  }

  // Process incoming requests from Modbus controller (Master)
  mb.task();

}
