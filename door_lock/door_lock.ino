/**************************************************************
 * This code controls the Home Manager's door lock,
 * on an ESP8266 microcontroller. It can also be opened
 * with another, magnetic key. More info:
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

const char * deviceName = "My Door lock";
const char * subscribedTopic = "my_door_lock";

#define TRIG_PIN 14

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
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
  digitalWrite(TRIG_PIN, HIGH);
  delay(50);
  digitalWrite(TRIG_PIN, LOW);
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
