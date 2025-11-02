#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <PubSubClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#define mqtt_server "192.168.1.64"
#define mqtt_user "mqtt_user"
#define mqtt_password ""

#define filter_pressure_diff "homeassistant/sensor/pressure"

//Modbus TCP server
ModbusIP mb;

//Register addresses
const uint16_t REG_TEMPERATURE = 100;
const uint16_t REG_HUMIDITY    = 101;

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

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

  client.setServer(mqtt_server, 1883);

  // Initialize Modbus TCP server
  mb.server(); //Start Modbus TCP server on port 502 (default)
  mb.addHreg(REG_TEMPERATURE, 0);
  mb.addHreg(REG_HUMIDITY, 0);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static uint32_t timer = millis();

  //Simulate values of sensor
  if (millis() - timer > 2000) {
    timer = millis();

    float temperature = 22.5 + (rand() % 100) / 10.0;
    float humidity = 40.0 + (rand() % 100) / 10.0;
    mb.Hreg(REG_TEMPERATURE, (uint16_t)(temperature * 10));
    mb.Hreg(REG_HUMIDITY, (uint16_t)(humidity * 10));

    Serial.printf("Updated: Temp = %.1f°C, Hum = %.1f%%\n", temperature, humidity);

    client.publish(filter_pressure_diff, String(REG_TEMPERATURE).c_str()); //This is for now for test, later I'll replace value REG_TEMPERATURE with real value from pressure sensor
    Serial.print("Published temperature: ");
    Serial.println(String(REG_TEMPERATURE).c_str());
  }

  // Process incoming requests from Modbus controller (Master)
  mb.task();

}
