// MyTempBridge.ino
// ESP32 + HomeSpan + Watchdog (5 min) — 4 virtual temperature sensors from CGMiner
// Wi-Fi: AirPort 

#include <WiFi.h>
#include <HomeSpan.h>
#include <stdlib.h>
#include <string.h>
#include "esp_task_wdt.h"

// === USER SETTINGS ===
const char* ssid     = "AirPort";
const char* password = "**********************************";
IPAddress hostIP(10,0,1,42);       // Avalon IP 
const uint16_t hostPort = 4028;    // Avalon port

const uint32_t INTERVAL_MS = 30000; // Poll interval (30 sec)
#define WDT_TIMEOUT_SEC  300         // 5 minutes

// === GLOBALS ===
uint32_t lastMillis = 0;
float mt1=0, mt2=0, mt3=0, tempAir=0;

// === HOMESPAN TEMP SENSOR ===
struct MyTempSensor : Service::TemperatureSensor {
  SpanCharacteristic* cTemp;
  float* src;
  bool init = false;
  MyTempSensor(float* ptr, const char* name) {
    src = ptr;
    new Characteristic::Name(name);
    cTemp = new Characteristic::CurrentTemperature();
  }
  void loop() override {
    float v = *src;
    if (!init || fabs(v - cTemp->getVal()) >= 0.1) {
      cTemp->setVal(v);
      init = true;
    }
  }
};

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(100);
  homeSpan.begin(Category::Bridges, "My Temp Bridge");

  new SpanAccessory(); new Service::AccessoryInformation(); new Characteristic::Identify();
  new SpanAccessory(); new Service::AccessoryInformation(); new Characteristic::Identify(); new MyTempSensor(&mt1, "Chip-One");
  new SpanAccessory(); new Service::AccessoryInformation(); new Characteristic::Identify(); new MyTempSensor(&mt2, "Chip-Two");
  new SpanAccessory(); new Service::AccessoryInformation(); new Characteristic::Identify(); new MyTempSensor(&mt3, "Chip-Three");
  new SpanAccessory(); new Service::AccessoryInformation(); new Characteristic::Identify(); new MyTempSensor(&tempAir, "Air-Inlet");

  // --- Setup Watchdog (WDT) for 5 minutes ---
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_SEC * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);  // Add current thread (loopTask)
}

void loop() {
  homeSpan.poll();
  esp_task_wdt_reset();

  static int failCount = 0;

  if (millis() - lastMillis > INTERVAL_MS) {
    lastMillis = millis();

    // Wi-Fi check and auto-reconnect
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.reconnect();
      delay(2000);
      return;
    }

    WiFiClient client;
    if (!client.connect(hostIP, hostPort)) {
      failCount++;
      if (failCount > 5) ESP.restart();
      return;
    } else {
      failCount = 0;
    }

    client.setTimeout(200);
    client.print("{\"command\":\"stats\"}\r\n");

    String resp;
    unsigned long tStart = millis();
    while (client.connected() && millis() - tStart < 800) {
      while (client.available()) resp += char(client.read());
    }
    client.stop();

    // --- Parse MTavg ---
    int p = resp.indexOf("MTavg[");
    if (p >= 0) {
      int e = resp.indexOf("]", p);
      String chunk = resp.substring(p + 6, e);
      char buf[64];
      chunk.toCharArray(buf, sizeof(buf));
      char* saveptr;
      char* tok = strtok_r(buf, " ", &saveptr);
      if (tok) mt1 = atof(tok);
      tok = strtok_r(NULL, " ", &saveptr);
      if (tok) mt2 = atof(tok);
      tok = strtok_r(NULL, " ", &saveptr);
      if (tok) mt3 = atof(tok);
    }

    // --- Parse Temp ---
    p = resp.indexOf("Temp[");
    if (p >= 0) {
      int e = resp.indexOf("]", p);
      tempAir = atof(resp.substring(p + 5, e).c_str());
    }
  }
}
