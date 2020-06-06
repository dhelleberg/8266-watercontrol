#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>

/*Put your SSID & Password*/
const char* ssid = WSSID;  // Enter SSID here
const char* password = WPWD;  //Enter Password here

const int WIFI_TIMEOUT_RESTART_S=60;
const int RECONNECT_INTERVAL = 30 * 1000;
long lastReconnect =  0;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  int mqtt_mode = payload.toInt();
  if (mqtt_mode == 1) {
    Serial.println("Water on...");
    digitalWrite(5, HIGH); 
  }
  if (mqtt_mode == 0) {
    Serial.println("Water off...");
    digitalWrite(5, LOW); 
  }
}



void setup() {

  Serial.begin(115200);
  pinMode(5, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to WiFi ");

  int wifi_connection_time = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wifi_connection_time++;
    if (wifi_connection_time > WIFI_TIMEOUT_RESTART_S) {
      Serial.println("[WIFI] Run into Wifi Timeout, try restart");
      Serial.flush();
      WiFi.disconnect();
      ESP.restart();
    }
  }
  Serial.println("\n[WIFI] Connected");
  lastReconnect = millis();

  Serial.print("\n mqtt.. connecting...");
  client.begin("192.168.178.21",net);
  client.onMessage(messageReceived);
  while (!client.connect("8266-water", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");
  client.subscribe("/water-status");
    // put your setup code here, to run once:
  

}

void loop() {
  client.loop();
  long now = millis();
  if(now > (lastReconnect + RECONNECT_INTERVAL)) {
    lastReconnect = now;
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WIFI re-connecting...");
      int wifi_retry = 0;
      while(WiFi.status() != WL_CONNECTED && wifi_retry < 5 ) {
        wifi_retry++;
        Serial.println("WiFi not connected. Try to reconnect");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        delay(100);
      }
    }
    if (!client.connected()) {
        Serial.println("mqtt client re-connecting...");
        int tries = 0;
        boolean repeat = true;
        while (!client.connect("arduino", "try", "try") && repeat) {
          Serial.print(".");
          delay(100);
          tries++;
          if(tries > 5)
            repeat = false;
        }
        Serial.println("reconnected: "+String(repeat));
        if(repeat)
          client.subscribe("/water-status");
    }  
  }

}