/**************************************************************
 * This code controls the Home Manager's robot arm,
 * it uses special messages, but it doesn't seem to
 * be a big issue. More info:
 * https://www.hackster.io/Abysmal/home-manager-db49c6
 * 
 * This project is made for "Unleash Invisible Intelligence"
 * contest on hackster.io. More info:
 * https://www.hackster.io/contests/maximunleash
 * 
 * author: Balázs Simon
 *
 **************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// don't leave it true if you don't use it
#define DEBUG false

// Update these with values suitable for your network.
const char* ssid = "your ssid";
const char* password = "password of your wifi network";
const char* mqtt_server = "mqtt server address";

const char* deviceName = "Robot Arm";
const char* subscribedTopic = "robot_arm";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(deviceName)) {
      if (DEBUG) Serial.println("connected");
      client.subscribe(subscribedTopic);
    } else {
      if (DEBUG) {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
