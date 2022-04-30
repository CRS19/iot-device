#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "NextCRSnet";
const char* password = "1725crs19";

const char* serverName = "http://192.168.101.2:3022/contacts/init";

const int PIN = 2;
const int CUTOFF = -60;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  
  pinMode(PIN, OUTPUT);
  BLEDevice::init("");
}

void loop() {

  StaticJsonDocument<200> doc;
  
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(1);
  int best = CUTOFF;
  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    Serial.println(results.getCount());
    char *a = "pepe";
    Serial.println("que jesto?");
    Serial.println(a);
    Serial.println(device.getName().c_str());
    doc["idDevice"] = device.getName().c_str();
    int rssi = device.getRSSI();
    if (rssi > best) {
      best = rssi;

      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      serializeJsonPretty(doc, Serial);
      String json;
      serializeJson(doc,json);
      
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(json);
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
    }
    Serial.println(rssi);
    delay(4000);
  }
  digitalWrite(PIN, best > CUTOFF ? HIGH : LOW);
}