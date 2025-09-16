#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>
// ESP-NOW
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// struct defnieren für Messwerte
typedef struct struct_message {
  float temperature;
  float humidity;
};

// Variable vom Typ struct_message deklarieren
struct_message dhtData;

// MAC-Addresse des Empfängers festlegen
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x0A, 0xFD, 0x5C};

// Callback, wenn Daten gesendet wurden
void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Sendestatus: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Erfolgreich" : "Fehlgeschlagen");
}


void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Starte DHT-Sensor...");
    dht.begin();
    // WLAN im Station-Mode starten
    WiFi.mode(WIFI_STA);
    // Kanal auf 1 setzen
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    // ESP-NOW starten und per Statuscode prüfen
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Fehler beim Initialisieren von ESP-NOW");
        return;
    }
    // Callback registrieren
    esp_now_register_send_cb(OnDataSent);
    // Empfänger hinzufügen
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Fehler beim Hinzufügen des Peers");
        return;
    }
}



void loop() {
  // --- Messwerte auslesen ---
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Fehler: Keine gültigen Daten vom DHT-Sensor!");
  } else {
    // In die Struktur speichern
    dhtData.temperature = t;
    dhtData.humidity = h;

    // Senden
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &dhtData, sizeof(dhtData));
    
    if (result == ESP_OK) {
      Serial.println("Daten gesendet:");
      Serial.print("  Temperatur: "); Serial.print(t); Serial.println(" °C");
      Serial.print("  Feuchtigkeit: "); Serial.print(h); Serial.println(" %");
    } else {
      Serial.println("Fehler beim Senden der Daten");
    }
  }




  delay(2000); // Wartezeit

}