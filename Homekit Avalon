// MyTempBridge.ino
// ESP32 + HomeSpan sketch to parse MTavg and Temp from CGMiner stats
// Replace placeholders before use

#include <WiFi.h>
#include <HomeSpan.h>
#include <stdlib.h>
#include <string.h>

// Wi-Fi credentials — set your own
const char* ssid     = "<YOUR_WIFI_SSID>";
const char* password = "<YOUR_WIFI_PASSWORD>";

// Remote CGMiner server — set correct IP and port
IPAddress hostIP(0,0,0,0);  // e.g., IPAddress(10,0,1,42)
const uint16_t hostPort = 4028;

// Poll interval (ms)
const uint32_t INTERVAL_MS = 30000;

// Temperature holders
uint32_t lastMillis = 0;
float mt1=0, mt2=0, mt3=0, tempAir=0;

// HomeKit temperature sensor accessory
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

  new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();

  new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new MyTempSensor(&mt1, "Chip-One");

  new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new MyTempSensor(&mt2, "Chip-Two");

  new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new MyTempSensor(&mt3, "Chip-Three");

  new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new MyTempSensor(&tempAir, "Air-Inlet");
}

void loop() {
  homeSpan.poll();

  if (millis() - lastMillis > INTERVAL_MS) {
    lastMillis = millis();

    WiFiClient client;
    if (!client.connect(hostIP, hostPort)) return;
    client.setTimeout(200);
    client.print("{\"command\":\"stats\"}\r\n");

    String resp;
    while (client.connected()) {
      while (client.available()) resp += char(client.read());
    }
    client.stop();

    // Parse MTavg
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

    // Parse Temp
    p = resp.indexOf("Temp[");
    if (p >= 0) {
      int e = resp.indexOf("]", p);
      tempAir = atof(resp.substring(p + 5, e).c_str());
    }
  }
}
