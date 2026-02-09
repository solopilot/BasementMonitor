// BasementMonitor
// See: https://pejaver.com/Projects/BasementMonitor.htm
// Set Tools -> Board -> ESP8266 -> Generic ESP8266 Module
// Set Flash size to: 4 MB (FS: 2MB, OTA ~1019KB)

#include <Wire.h>

float temperature_BMP280;
float temperature_BMP280F;
float pressure;
float pressureHg;

//AHT20
float temperature_AHT20;
float temperature_AHT20F;
float humidity;

const float hPaToInHg = 0.02952998057228;   // conversion factor
const bool verbose = false;

//  How often to check for new values
unsigned long HeartbeatMillis = 0;
const long Heartbeatinterval = 5000;  // read new values every 5 secs

int LED2 = 16;        // D0, GPIO 16, 1st pin on right, blue center LED
int LED = 2;          // D4, GPIO 2, 5th pin on right, blue LED

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  setupSensor();
  Wire.begin();
  AHT20_begin();
  BMP280_begin();
  startMeasurementAHT20();
  setupWiFi();
  setupEmail();
}

void loop() {
  checkbusyAHT20();
  getDataAHT20();

  unsigned long currentMillis = millis();
  if (currentMillis - HeartbeatMillis >= Heartbeatinterval) {
    HeartbeatMillis = currentMillis;
    digitalWrite(LED, LOW);         // LED on

    //BMP280
    readTemperatureBMP280();
    temperature_BMP280F = temperature_BMP280*9./5.+32.;
    readPressureBMP280();
    pressureHg = pressure*hPaToInHg;
    startMeasurementAHT20();
    temperature_AHT20F = temperature_AHT20*9./5.+32.;

    loopSensor();        // read water sensor
    loopEmail();         // send alerts if needed

    if (verbose) {
      Serial.print("temperature_BMP280: ");
      Serial.print(temperature_BMP280);
      Serial.print(" C,  ");
      Serial.print(temperature_BMP280F);
      Serial.println(" F");

      Serial.print("Pressure BMP280: ");
      Serial.print(pressure);
      Serial.print(" hPa  ");;
      Serial.print(pressureHg);
      Serial.println(" inHg");

      Serial.print("Humidity AHT20: ");
      Serial.print(humidity);
      Serial.println(" %");

      Serial.print("Temperature AHT20: ");
      Serial.print(temperature_AHT20);
      Serial.print(" C,  ");
      Serial.print(temperature_AHT20F);
      Serial.println(" F");

      Serial.println("----------------------- ");
    }
    digitalWrite(LED, HIGH);    // LED off
  }

  loopWiFi();   // check for web request
}
