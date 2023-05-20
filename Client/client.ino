#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* thisIdDevice = "SDM-001";
const char* ssid = "FIBRAMAX_CRISTIAN";
const char* password = "1725crs19";
const char* serverName = "http://192.168.123.102:3022/contacts/init";
const char* thisDeviceNameCode = "SDM-";
const char* JWT = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjp7Il9pZCI6IjY0NjhhZThmYWI0NGM1YzYzZjQ2ZjdiOSIsImZ1bGxOYW1lIjoiRGV2aWNlQWNjb3VudCIsImlkRGV2aWNlIjoiaWREZXZpY2UiLCJpc0RldmljZSI6dHJ1ZSwiaXNQb3NzaWJsZVNpY2siOmZhbHNlLCJpc1NpY2siOmZhbHNlLCJtYWlsIjoiZGV2aWNlQGEuY29tIiwicm9sIjoiREVWSUNFIiwibmVhck5vZGVzIjpbXSwiX192IjowfSwiaWF0IjoxNjg0NTgyNDA1LCJleHAiOjE2ODQ2Njg4MDV9.UpoOtLvVO7VxiTs4yW7vNi6ZObVM7t0ujlTPdNOFKzw";
const double A = -66.5;
const double n = 2.6;
bool devicesMemory[200] = {0};

const int SOUND_PIN = 23;

const int DISTANCE_LIMIT = 1.5;

void setup() {
  Serial.begin(115200);
  Serial.println("Soy el dispositivo cliente " + String(thisIdDevice));

  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); 
  pinMode(SOUND_PIN, OUTPUT);
  BLEDevice::init("");
}

void loop() {

  StaticJsonDocument<200> doc;
  
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(1);
  Serial.println("********************* Count: " + String(results.getCount()) + " ************************");  
  Serial.println(results.getCount());

  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);
    const char* contactDeviceName = device.getName().c_str();
    char codeContactDeviceName[4];
    int rssi = device.getRSSI();
    int isTheSameDevice = strcmp(thisIdDevice, contactDeviceName);
    strncpy(codeContactDeviceName, contactDeviceName, 4);
    codeContactDeviceName[4] = '\0';
    int isTheSameCode = strcmp(thisDeviceNameCode, codeContactDeviceName);
    
    double distance = calculateDistance(rssi);
    
    Serial.print(i+1);
    Serial.print(": ");
    Serial.println(device.getName().c_str());
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.print(" -> Distance: ");
    Serial.println(distance);
    Serial.println("Es el mismo dispositivo -> " + String(isTheSameDevice));

    

    if (isTheSameDevice != 0 && isTheSameCode == 0) {
      char numberContactDeviceName[4];
      substring(contactDeviceName, 4, 6, numberContactDeviceName);
      int numberContactDeviceNameInt = atoi(numberContactDeviceName);
      Serial.println("numberContactDeviceName: " + String(numberContactDeviceName));
      Serial.println("number transformed to int -> " + String(numberContactDeviceNameInt));
      Serial.print("devicesMemory[numberContactDeviceNameInt] -> ");
      Serial.println(devicesMemory[numberContactDeviceNameInt]);

      if(distance < DISTANCE_LIMIT) {
        Serial.println("------------------------------------- DISTANCIA MENOR A 1.5m -----------------------------------------------");
        
        if (devicesMemory[numberContactDeviceNameInt] == 1) {
          Serial.println("Dispisitivo ya registrado");
          Serial.println("[STEP]> Emitiendo alarma sonora");
          emitTwoBeeps();

        } else {
          Serial.println("[STEP]> Emitiendo alarma sonora");
          emitTwoBeeps();
          Serial.println("[STEP]> Enviar datos via POST init TRUE");
        
          Serial.println("Este si esta dentro del rango");
          WiFiClient client;
          HTTPClient http;

          Serial.print("[PROCESSING]: ");
          Serial.println(device.getName().c_str());
          doc["idDevice"] = String(thisIdDevice);
          doc["idContactDevice"] = String(device.getName().c_str());
          doc["rssi"] = String(rssi);
          doc["isInit"] = true;
      
          http.begin(client, serverName);

          serializeJsonPretty(doc, Serial);
          Serial.println();
          String json;
          serializeJson(doc,json);
          
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", String(JWT));
          int httpResponseCode = http.POST(json);
        
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);

          Serial.println("[STEP]> Guardar en memoria");
          devicesMemory[numberContactDeviceNameInt] = 1;
        } 
     
      } else {
        Serial.println("------------------------------------- DISTANCIA MAYOR A 1.5m -----------------------------------------------");
        
        if (devicesMemory[numberContactDeviceNameInt] == 1) {
          Serial.println("[STEP]> Enviar datos via POST init FALSE");
        
          Serial.println("Este si esta dentro del rango");
          WiFiClient client;
          HTTPClient http;

          Serial.print("[PROCESSING]: ");
          Serial.println(device.getName().c_str());
          doc["idDevice"] = String(thisIdDevice);
          doc["idContactDevice"] = String(device.getName().c_str());
          doc["rssi"] = String(rssi);
          doc["isInit"] = false;
      
          http.begin(client, serverName);

          serializeJsonPretty(doc, Serial);
          Serial.println();
          String json;
          serializeJson(doc,json);
          
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", String(JWT));
          int httpResponseCode = http.POST(json);
        
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);

          Serial.println("[STEP]> Eliminar dispositivo en memoria");
          devicesMemory[numberContactDeviceNameInt] = 0;
        }
      }
    }
  }
  delay(4000);
}

void emitTwoBeeps() {
  tone(SOUND_PIN, 1000);  
  delay(100);             
  noTone(SOUND_PIN);      
  
  delay(100);             
  
  tone(SOUND_PIN, 1000);  
  delay(100);             
  noTone(SOUND_PIN);      
}

void substring(const char* str, int start, int length, char* result) {
    const char* sourcePtr = str + start;
    char* destinationPtr = result;
    
    for (int i = 0; i < length; i++) {
        *destinationPtr = *sourcePtr;
        sourcePtr++;
        destinationPtr++;
    }
    
    *destinationPtr = '\0';
}

double calculateDistance(int rssi) {
  double distance = pow(10, - ((rssi - A) / (10 * n)));

  return distance;
}