// Sump water sensor

String waterText = "";    // text of waterMsg without the analog value
int severity = 0;        // 0: all OK, 1: equipment problem, 2: water alert, 3: oh shit
String waterMsg = "";     // this is displayed by web server
int POWER = 10;     // SD3, 4th pin on left, GPIO pin provides switched power for water sensor

void setupSensor() {
  pinMode(POWER, OUTPUT);
}

// read water sensor
// this should be called once every few (5) secs
void loopSensor() {
  digitalWrite(POWER, HIGH);      // provide power for sensor

  // delay to allow water to ionize, or something, seems silly
  delay(100);                     // setting this to >200ms causes the ATH20 to fail

  int sensorValue = analogRead(A0);
  if (sensorValue > 1020) {
    waterText = "sensor is not connected!";
    severity = 1;
  }
  else if (sensorValue > 800) {
    waterText = "level is OK";
    severity = 0;
  }
  else if (sensorValue > 400) {
    waterText = "level is HIGH!!";
    severity = 2;
  }
  else if (sensorValue > 20) {
    waterText = "level is VERY HIGH!!";
    severity = 3;
  }
  else {    // range(0, 20)
    waterText = "sensor is shorted!";
    severity = 1;
  }

  waterMsg = "Sump water " + waterText + " (" + sensorValue + ")";

  digitalWrite(POWER, LOW);
}
