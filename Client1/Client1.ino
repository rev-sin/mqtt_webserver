#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi Credentials
const char* ssid = "RevSinMax";       // your WiFi SSID
const char* password = "alrightokay2";   // your WiFi Password

// MQTT Broker
const char* mqtt_server = "172.20.10.3";  // replace with your broker IP

WiFiClient espClient;
PubSubClient client(espClient);

// DS18B20 setup
#define ONE_WIRE_BUS 4   // GPIO pin connected to DS18B20 data pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Simulated DH11 (bee count sensor substitute)
#define BEE_SENSOR 34   // Analog input pin
int beeCount = 0;

void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.print("Message from server: ");
  Serial.println(msg);

  if (msg == "expand the Hive") {
    Serial.println(">>> ACTION: EXPAND THE HIVE <<<");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client1")) {
      Serial.println("connected");
      client.subscribe("hive/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // Simulate bee count (0–20 range from analog sensor)
  int sensorValue = analogRead(BEE_SENSOR);
  beeCount = map(sensorValue, 0, 4095, 0, 20);

  // Create JSON payload
  String payload = "{";
  payload += "\"temperature\":";
  payload += temperatureC;
  payload += ",\"beeCount\":";
  payload += beeCount;
  payload += "}";

  client.publish("hive/data", payload.c_str());
  Serial.println("Published: " + payload);

  delay(5000); // update every 5 seconds
}
