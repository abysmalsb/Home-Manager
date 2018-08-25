#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SENSOR_POWER_PIN  4   // D2
#define SENSOR            A0
#define PUMP_PIN          5   // D1
#define IRRIGATION_TIME   1500

const char* ssid = "your ssid";
const char* password = "password of your wifi network";
const char* mqtt_server = "mqtt server address";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup() {
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
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
    digitalWrite(PUMP_PIN, HIGH);
    delay(IRRIGATION_TIME);
    digitalWrite(PUMP_PIN, LOW);
  } 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Mimosa")) {
      Serial.println("connected"); \
      client.subscribe("irrigating_mimosa");
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

  long now = millis();
  if (now - lastMsg > 60000) {
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(10);
    
    lastMsg = now;
    String msg = String(analogRead(SENSOR));
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("mimosa_moisture_level", s2c(msg));
    
    digitalWrite(SENSOR_POWER_PIN, LOW);
  }
}

char* s2c(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}
