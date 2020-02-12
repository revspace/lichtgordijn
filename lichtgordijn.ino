#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define PIN_FLANK D1

#define WIFI_SSID "revspace-pub-2.4ghz"
#define WIFI_PASS ""

#define MQTT_HOST "mosquitto.space.revspace.nl"
#define MQTT_TOPIC "revspace/lichtgordijn"

WiFiClient espClient;
PubSubClient client(espClient);

int state = 2; // impossible state
int connect_counter = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup...");

  pinMode(PIN_FLANK, INPUT_PULLUP);
  
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(MQTT_HOST, 1883);

  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    // Attempt to connect
    if (client.connect("lichtgordijn")) {
      Serial.println("Connected");
    } else {
      connect_counter++;
      if(connect_counter > 3) {
        Serial.println("Failed >3 times, restarting");
        ESP.restart();
      }
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void loop() {
    if (!client.connected()) {
      Serial.println("Connection lost, reconnecting...");
      reconnect();
    }
    client.loop();

    int current = digitalRead(PIN_FLANK);
    
    if(current != state) {
      if (current == LOW) client.publish(MQTT_TOPIC, "TRIGGER", true);
      if (current == HIGH) client.publish(MQTT_TOPIC, "IDLE", true);
    }
    state = current;
    delay(100);
}
