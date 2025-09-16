#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>


// GPIO auf dem C3 für den DHT11
#define DHTPIN 5
// DHT-Typ definieren
#define DHTTYPE DHT11
// dht.-Objekt erzeugen mit defines
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starte DHT-Sensor...");
  dht.begin();
}

void loop() {
  // Messwerte auslesen
  float t = dht.readTemperature(); // Temperature
  float h = dht.readHumidity(); // Luftfeuchtigkeit

  // --- Fehlerbehandlung ---
  if (isnan(t) || isnan(h)) {
    Serial.println("Fehler: Keine gültigen Daten vom DHT-Sensor!");
  } else {
    // Ausgabe
    Serial.print("Temperatur: ");
    Serial.print(t);
    Serial.println(" °C");

    Serial.print("Luftfeuchtigkeit: ");
    Serial.print(h);
    Serial.println(" %");
  }

  delay(2000);
}
