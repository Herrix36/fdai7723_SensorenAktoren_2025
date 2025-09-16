#include <esp_now.h>
#include <WiFi.h>

// Beispielstruktur für empfangene Daten
typedef struct struct_message {
  float temperatur;
  float feuchtigkeit;
} struct_message;

// Empfangene Daten speichern
struct_message incomingData;

// Callback-Funktion bei Datenempfang
void OnDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  // Mac des Absenders auslesen
  Serial.print("Empfangen von: ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "b0:81:84:04:62:28",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
  
  
  Serial.print("Temperatur: ");
  Serial.println(incomingData.temperatur);
  Serial.print("Feuchtigkeit: ");
  Serial.println(incomingData.feuchtigkeit);
  Serial.println("-------------------------");
}

void setup() {
  Serial.begin(115200);

  // WiFi auf "Station Mode"
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Sicherstellen, dass keine Verbindung besteht
  delay(100);

  // ESP-NOW initialisieren
  if (esp_now_init() != ESP_OK) {
    Serial.println("Fehler beim Initialisieren von ESP-NOW");
    return;
  }

  // Callback registrieren
  esp_now_register_recv_cb(OnDataRecv);
  // Debug-Info
  Serial.println("ESP-NOW Empfänger bereit");
}

void loop() {
  // nichts tun – warten auf Daten
}