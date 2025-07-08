#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include "BMA250.h"
#include <ArduinoLowPower.h>  // Low-power sleep

const int chipSelect = 10;
File dataLogFile;
File startupLogFile;
File errorLogFile;

BMA250 accel;
RTC_DS1307 rtc;

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
  }
}

void getTimestamp(char* dateStr, size_t dateLen, char* timeStr, size_t timeLen) {
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    snprintf(dateStr, dateLen, "%04d-%02d-%02d", now.year(), now.month(), now.day());
    snprintf(timeStr, timeLen, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  } else {
    snprintf(dateStr, dateLen, "0000-00-00");
    snprintf(timeStr, timeLen, "00:00:00");
  }
}

void logStartup(const char* message) {
  if (!startupLogFile) return;
  char dateStr[11], timeStr[9];
  getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));
  startupLogFile.print(dateStr); startupLogFile.print(","); startupLogFile.print(timeStr);
  startupLogFile.print(",STARTUP,"); startupLogFile.println(message);
  startupLogFile.flush();
}

void logError(const char* message) {
  if (!errorLogFile) return;
  char dateStr[11], timeStr[9];
  getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));
  errorLogFile.print(dateStr); errorLogFile.print(","); errorLogFile.print(timeStr);
  errorLogFile.print(",ERROR,"); errorLogFile.println(message);
  errorLogFile.flush();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(3, 200);

  SerialUSB.begin(115200);
  delay(3000);

  Wire.begin();
  blinkLED(1, 300);

  if (!rtc.begin()) {
    blinkLED(5, 300);
    while (1);
  }
  blinkLED(2, 300);

  if (!SD.begin(chipSelect)) {
    blinkLED(5, 300);
    while (1);
  }
  blinkLED(3, 300);

  if (!SD.exists("/d")) {
    if (!SD.mkdir("/d")) {
      blinkLED(5, 300);
      while (1);
    }
    blinkLED(4, 300);
  }

  startupLogFile = SD.open("/d/startup.log", FILE_WRITE);
  if (!startupLogFile) {
    blinkLED(5, 300);
    while (1);
  }
  logStartup("Logger setup started");

  errorLogFile = SD.open("/d/errors.log", FILE_WRITE);
  if (!errorLogFile) {
    blinkLED(5, 300);
    while (1);
  }

  logStartup("Wire initialized");
  logStartup("RTC initialized");
  logStartup("SD card initialized");

  DateTime now = rtc.now();
  char dataFilename[20];
  snprintf(dataFilename, sizeof(dataFilename), "/d/log%02d%02d.csv", now.month(), now.day());
  bool newFile = !SD.exists(dataFilename);
  dataLogFile = SD.open(dataFilename, FILE_WRITE);
  if (!dataLogFile) {
    logError("Failed to open data log file");
    blinkLED(5, 300);
    while (1);
  }
  if (newFile) {
    dataLogFile.println("Date,Time,X,Y,Z,Temp(C)");
    dataLogFile.flush();
  }
  logStartup("Data log file opened");

  accel.begin(BMA250_range_2g, BMA250_update_time_64ms);
  blinkLED(3, 100);
  logStartup("Accelerometer initialized");
  logStartup("Setup complete, entering main loop.");
}

void loop() {
  accel.read();
  int x = accel.X;
  int y = accel.Y;
  int z = accel.Z;
  double temp = accel.rawTemp * 0.5 + 24.0;

  if (x == -1 && y == -1 && z == -1) {
    logError("BMA250 NOT DETECTED");
    blinkLED(5, 200);
    while (1);
  }

  char dateStr[11], timeStr[9];
  getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));

  SerialUSB.print("Log: "); SerialUSB.print(dateStr); SerialUSB.print(" | ");
  SerialUSB.print(timeStr); SerialUSB.print(" | X="); SerialUSB.print(x);
  SerialUSB.print(" Y="); SerialUSB.print(y); SerialUSB.print(" Z=");
  SerialUSB.print(z); SerialUSB.print(" | Temp="); SerialUSB.println(temp, 2);

  dataLogFile.print(dateStr); dataLogFile.print(",");
  dataLogFile.print(timeStr); dataLogFile.print(",");
  dataLogFile.print(x); dataLogFile.print(",");
  dataLogFile.print(y); dataLogFile.print(",");
  dataLogFile.print(z); dataLogFile.print(",");
  dataLogFile.print(temp, 2); dataLogFile.println();
  dataLogFile.flush();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(150);
  digitalWrite(LED_BUILTIN, LOW);

  LowPower.sleep(7000);  // Sleep for 7 seconds (Allows a 10 second interval)
}
