#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Gsender.h"

#define DEBUG false

const char* ssid = "your ssid";
const char* password = "password of your wifi network";
const char* mqtt_server = "mqtt server address";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
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
  Serial.print("{\"topic\":\"");
  Serial.print(topic);
  Serial.print("\",\"message\":\"");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("\"}");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (DEBUG) Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      if (DEBUG) Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("my_door_lock", "hello world");
      // ... and resubscribe
      client.subscribe("mimosa_humidity_level");
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

  if (Serial.available()) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial.readStringUntil('\n'));

    String type = root["type"];
    if (type.equals("mqtt")) {
      if (DEBUG) {
        Serial.println("publishing message:");
        Serial.print("topic: ");
        root["topic"].printTo(Serial);
        Serial.println();
        Serial.print("message: ");
        root["message"].printTo(Serial);
        Serial.println();
      }
      client.publish(root["topic"], root["message"]);
    }
    else if (type.equals("email")) {
      Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
      String subject = root["subject"];
      if (gsender->Subject(subject)->Send(root["address"], root["message"])) {
        if (DEBUG) Serial.println("{\"status\":\"Success\"}");
      } else {
        if (DEBUG) Serial.print("{\"status\":\"Fail\",\"error:\":\"");
        if (DEBUG) Serial.print(gsender->getError());
        if (DEBUG) Serial.println("\"}");
      }
    }
  }
}

char* s2c(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}


