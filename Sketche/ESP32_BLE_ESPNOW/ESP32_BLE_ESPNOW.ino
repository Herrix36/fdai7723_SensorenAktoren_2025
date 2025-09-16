#include <esp_now.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_wifi.h>

// UUIDs für BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// BLE Variablen
BLECharacteristic *pCharacteristic;
BLE2902 *p2902Descriptor;
bool deviceConnected = false;

// Struktur für empfangene Sensordaten
typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

struct_message cachedData;   // Zwischengespeicherte Daten
volatile bool newDataAvailable = false;

// ESP-NOW Callback 
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  if (len != sizeof(cachedData)) return;

  memcpy(&cachedData, incomingDataBytes, sizeof(cachedData));
  newDataAvailable = true;

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  Serial.printf("Von %s empfangen: T=%.1f°C H=%.1f%%\n",
                macStr, cachedData.temperature, cachedData.humidity);
}

// BLE Server Callbacks 
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("BLE Gerät verbunden");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("BLE Gerät getrennt");
  }
};

//ESP-NOW Start 
void startESPNOW() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Fehler bei ESPNOW Init");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  // Fester Kanal, damit BLE/ESP-NOW stabil laufen
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);

  Serial.println("ESPNOW gestartet und Kanal gesetzt");
}

// BLE Start
void startBLE() {
  Serial.println("Starte BLE...");

  BLEDevice::init("ESP32_Sensor");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );
  p2902Descriptor = new BLE2902();
  p2902Descriptor->setNotifications(true);
  pCharacteristic->addDescriptor(p2902Descriptor);

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->start();

  Serial.println("BLE Advertising gestartet, warte auf Verbindung...");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starte ESP-NOW + BLE Gateway...");

  // Zuerst BLE starten
  startBLE();
  delay(500); // kurze Pause für BLE-Stack

  // Danach ESP-NOW aktivieren
  startESPNOW();
}

void loop() {
  // Wenn neue Daten vorhanden sind und ein BLE-Client verbunden ist → senden
  if (newDataAvailable && deviceConnected) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f,%.1f", cachedData.temperature, cachedData.humidity);
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();

    Serial.print("BLE gesendet: ");
    Serial.println(buffer);

    newDataAvailable = false; // Flag zurücksetzen
  }

  delay(1); // wichtig: CPU-Zeit für BLE/WiFi Tasks freigeben
}
