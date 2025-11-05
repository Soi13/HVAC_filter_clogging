#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <PubSubClient.h>

const char* ssid = "Soi13";
const char* password = "";

#define mqtt_server "192.168.1.64"
#define mqtt_user "mqtt_user"
#define mqtt_password ""

#define filter_pressure_diff "homeassistant/sensor/pressure"

//Modbus TCP server
ModbusIP mb;

//Register addresses
const uint16_t REG_TEMPERATURE = 100;
const uint16_t REG_HUMIDITY    = 102;

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
  mb.addHreg(REG_TEMPERATURE, 0, 2);
  mb.addHreg(REG_HUMIDITY, 0, 2);
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

    //Store float -> two 16-bit registers
    uint16_t *t = (uint16_t*)&temperature;
    uint16_t *h = (uint16_t*)&humidity;

    mb.Hreg(REG_TEMPERATURE, t[0]);
    mb.Hreg(REG_TEMPERATURE + 1, t[1]);
    mb.Hreg(REG_HUMIDITY, h[0]);
    mb.Hreg(REG_HUMIDITY + 1, h[1]);

    Serial.printf("Updated: Temp = %.1f°C, Hum = %.1f%%\n", temperature, humidity);

    client.publish(filter_pressure_diff, String(temperature).c_str()); //This is for now for test, later I'll replace value REG_TEMPERATURE with real value from pressure sensor
    Serial.print("Published temperature: ");
    Serial.println(String(temperature).c_str());
  }

  // Process incoming requests from Modbus controller (Master)
  mb.task();

}
