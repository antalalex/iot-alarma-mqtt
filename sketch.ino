#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===================== WiFi (Wokwi) =====================
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ===================== HiveMQ Cloud  =====================
const char* MQTT_HOST = "ad777200c8b94125895879226093bfba.s1.eu.hivemq.cloud"; 
const int   MQTT_PORT = 8883;                          
const char* MQTT_USER = "esp32";               
const char* MQTT_PASS = "Esp32pass";                 

// ===================== Topicuri MQTT =====================
const char* TOPIC_EVENT  = "iot/alarma/event";   // motion_detected / motion_cleared
const char* TOPIC_PIR    = "iot/alarma/pir";     // 0/1
const char* TOPIC_STATUS = "iot/alarma/status";  // online + armed/alarm
const char* TOPIC_CMD    = "iot/alarma/cmd";    

// ===================== Pini (Wokwi) =====================
// PIR: VCC->5V, GND->GND, OUT->GPIO15
// LED: A->GPIO2, C->GND
// Buzzer: BZ1 1->GPIO4, BZ1 2->GND
const int PIR_PIN    = 15;
const int LED_PIN    = 2;
const int BUZZER_PIN = 4;

WiFiClientSecure net;
PubSubClient mqtt(net);

bool armed = true;
bool alarmActive = false;
bool lastPir = false;

void publishStatus() {
  char msg[64];
  snprintf(msg, sizeof(msg), "armed=%d alarm=%d", armed ? 1 : 0, alarmActive ? 1 : 0);
  mqtt.publish(TOPIC_STATUS, msg, true);
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String cmd;
  for (unsigned int i = 0; i < length; i++) cmd += (char)payload[i];
  cmd.trim();
  cmd.toUpperCase();

  if (t == TOPIC_CMD) {
    if (cmd == "ARM") {
      armed = true;
      publishStatus();
      Serial.println("CMD: ARM");
    } else if (cmd == "DISARM") {
      armed = false;
      alarmActive = false;
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
      publishStatus();
      Serial.println("CMD: DISARM");
    }
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");
}

void connectMQTT() {
  // În Wokwi nu validăm certificatele TLS -> setInsecure()
  net.setInsecure();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32-alarma-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("MQTT connecting...");
    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_CMD);

      mqtt.publish(TOPIC_STATUS, "online", true);
      publishStatus();
      mqtt.publish(TOPIC_PIR, "0", true);
    } else {
      Serial.println("FAILED, retry...");
      delay(800);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  connectWiFi();
  connectMQTT();

  Serial.println("MQTT conectat cu succes. Sistem configurat cu succes.");
Serial.println("Sistem de alarma ACTIV.");
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  bool pir = (digitalRead(PIR_PIN) == HIGH);

  if (pir != lastPir) {
    lastPir = pir;
    mqtt.publish(TOPIC_PIR, pir ? "1" : "0", true);
    Serial.print("PIR=");
    Serial.println(pir ? "1" : "0");
  }

  if (!armed) {
    delay(50);
    return;
  }

  if (pir && !alarmActive) {
    alarmActive = true;
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);

    mqtt.publish(TOPIC_EVENT, "ALERTA: Miscare in zona monitorizata", false);
    publishStatus();
    Serial.println("ALERTA: Miscare in zona monitorizata");
  }

  if (!pir && alarmActive) {
    alarmActive = false;
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);

    mqtt.publish(TOPIC_EVENT, "Alarma oprita", false);
    publishStatus();
    Serial.println("Alarma oprita.");
  }

  delay(50);
}
