#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_SCD30.h>
#include <Bounce2.h>
#include <TFT_eSPI.h>

// BME680 Sensor-Objekt
Adafruit_BME680 bme;

// SCD30 Sensor-Objekt
Adafruit_SCD30 scd30;

// TFT-Display
TFT_eSPI tft = TFT_eSPI();

// Taster-Pins
const int buttonAll = 25;
const int buttonBME = 26;
const int buttonSCD = 27;
const int buttonKY = 14;

// KY-037 Mikrofon-Pin
const int kyPin = 34;

// Debounce-Objekte für Taster
Bounce debouncerAll = Bounce();
Bounce debouncerBME = Bounce();
Bounce debouncerSCD = Bounce();
Bounce debouncerKY = Bounce();

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // TFT-Display initialisieren
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("System startet...");

  // BME680 initialisieren
  if (!bme.begin(0x77)) {
    Serial.println("Fehler: BME680 nicht gefunden!");
    tft.println("BME680 Fehler!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  // SCD30 initialisieren
  if (!scd30.begin()) {
    Serial.println("Fehler: SCD30 nicht gefunden!");
    tft.println("SCD30 Fehler!");
    while (1);
  }
  
  // Taster als Eingänge konfigurieren mit internen Pull-Ups
  pinMode(buttonAll, INPUT_PULLUP);
  pinMode(buttonBME, INPUT_PULLUP);
  pinMode(buttonSCD, INPUT_PULLUP);
  pinMode(buttonKY, INPUT_PULLUP);

  // Debounce-Objekte mit den Pins verbinden
  debouncerAll.attach(buttonAll, INPUT_PULLUP);
  debouncerBME.attach(buttonBME, INPUT_PULLUP);
  debouncerSCD.attach(buttonSCD, INPUT_PULLUP);
  debouncerKY.attach(buttonKY, INPUT_PULLUP);

  debouncerAll.interval(50);
  debouncerBME.interval(50);
  debouncerSCD.interval(50);
  debouncerKY.interval(50);

  // KY-037 Mikrofon
  pinMode(kyPin, INPUT);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.println("Bereit!");
}

void loop() {
  // Aktualisiert die Taster-States
  debouncerAll.update();
  debouncerBME.update();
  debouncerSCD.update();
  debouncerKY.update();

  // Prüft, ob ein Taster gedrückt wurde
  if (debouncerAll.fell()) {
    measureAllSensors();
  }
  if (debouncerBME.fell()) {
    measureBME();
  }
  if (debouncerSCD.fell()) {
    measureSCD30();
  }
  if (debouncerKY.fell()) {
    measureKY037();
  }
}

// Funktion: Alle Sensoren messen
void measureAllSensors() {
  tft.fillScreen(TFT_BLACK);
  measureBME();
  measureSCD30();
  measureKY037();
}

// Funktion: BME680 messen
void measureBME() {
  if (!bme.performReading()) {
    Serial.println("Fehler beim Lesen des BME680!");
    return;
  }

  float temp = bme.temperature;
  float hum = bme.humidity;
  float pres = bme.pressure / 100.0;

  Serial.print("BME680 - Temperatur: ");
  Serial.print(temp);
  Serial.println(" °C");
  Serial.print("BME680 - Luftfeuchtigkeit: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("BME680 - Druck: ");
  Serial.print(pres);
  Serial.println(" hPa");

  tft.setCursor(0, 0);
  tft.println("BME680:");
  tft.printf("Temp: %.2f C\n", temp);
  tft.printf("Hum: %.2f %%\n", hum);
  tft.printf("Pres: %.2f hPa\n", pres);
}

// Funktion: SCD30 messen
void measureSCD30() {
  if (scd30.dataReady()) {
    float co2 = scd30.CO2;
    float temp = scd30.temperature;
    float hum = scd30.relative_humidity;

    Serial.print("SCD30 - CO2: ");
    Serial.print(co2);
    Serial.println(" ppm");
    Serial.print("SCD30 - Temperatur: ");
    Serial.print(temp);
    Serial.println(" °C");
    Serial.print("SCD30 - Luftfeuchtigkeit: ");
    Serial.print(hum);
    Serial.println(" %");

    tft.setCursor(0, 80);
    tft.println("SCD30:");
    tft.printf("CO2: %.2f ppm\n", co2);
    tft.printf("Temp: %.2f C\n", temp);
    tft.printf("Hum: %.2f %%\n", hum);
  } else {
    Serial.println("SCD30 - Keine Daten verfügbar");
  }
}

// Funktion: KY-037 messen
void measureKY037() {
  int micValue = analogRead(kyPin);

  Serial.print("KY-037 - Lautstärke: ");
  Serial.println(micValue);

  tft.setCursor(0, 160);
  tft.println("KY-037:");
  tft.printf("Mic: %d\n", micValue);
}
