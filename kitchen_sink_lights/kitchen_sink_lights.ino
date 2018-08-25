/**************************************************************
 * This code controls the Home Manager's following module:
 * LED lights at the kitchen sink. More info:
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

#define BUTTON            13
#define LED_LIGHTING      12

// Update these with values suitable for your network.
const char* ssid = "your ssid";
const char* password = "password of your wifi network";
const char* mqtt_server = "mqtt server address";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
int ledState = LOW;
int previousButtonState = LOW;

void setup() {
  pinMode(BUTTON, INPUT);            // Initialize the BUTTON pin as an input
  pinMode(LED_LIGHTING, OUTPUT);     // Initialize the LED_LIGHTING pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_LIGHTING, HIGH);
    ledState = HIGH;
  } else {
    digitalWrite(LED_LIGHTING, LOW);
    ledState = LOW;
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Kitchen Sink Lights")) {
      Serial.println("connected");
      client.subscribe("kitchen_sink_lights");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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

  int buttonState = digitalRead(BUTTON);
  if (buttonState) {
    if (!previousButtonState) {
      ledState = !ledState;
      digitalWrite(LED_LIGHTING, ledState);
      previousButtonState = HIGH;
      delay(50);
    }
  }
  else {
    previousButtonState = LOW;
  }
}
