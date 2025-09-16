#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
BLE2902 *p2902Descriptor;   // Descriptor, um zu prüfen ob Notifications aktiv sind
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Gerät verbunden.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Gerät getrennt.");
    delay(500);  // kurze Pause für Browser
    pServer->getAdvertising()->start();
    Serial.println("Werbung neu gestartet.");
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starte BLE-Server...");

  // BLE initialisieren
  BLEDevice::init("ESP32_BLE_Device");
  // BLE Server erzeugen
  BLEServer *pServer = BLEDevice::createServer(); 
  //Callback-Klasse zuweisen
  pServer->setCallbacks(new MyServerCallbacks());
  
  // BLE Service mit UUID erstellen
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Charakteristik mit READ + NOTIFY
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | // Client kann den Wert diekt abfragen
    BLECharacteristic::PROPERTY_NOTIFY // Server kann Änderunge an Clients pushen
  );

  // Descriptor hinzufügen (BLE Standart)
  p2902Descriptor = new BLE2902();
  p2902Descriptor->setNotifications(false);
  pCharacteristic->addDescriptor(p2902Descriptor);

  // Startwert setzen
  pCharacteristic->setValue("0.0,0.0");

  // Service starten
  pService->start();

  // Advertising konfigurieren
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);

  // Stabileres Connection-Interval
  pAdvertising->setMinPreferred(0x06);  // 30 ms
  pAdvertising->setMaxPreferred(0x12);  // 90 ms

  pAdvertising->start();
  Serial.println("BLE Server gestartet – Werbung läuft...");
  Serial.println("Warte auf Bluetooth-Verbindung...");
}

void loop() {
  if (deviceConnected && p2902Descriptor->getNotifications()) {
    // Beispielwerte (Temp. & Luftfeuchtigkeit)
    float temp = random(200, 300) / 10.0;  // 20.0 - 30.0 °C
    float hum  = random(400, 700) / 10.0;  // 40.0 - 70.0 %

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f,%.1f", temp, hum);

    // Wert setzen und senden
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();

    // Seriell ausgeben
    Serial.print("Gesendet: ");
    Serial.println(buffer);

    delay(2000);
  }
}
