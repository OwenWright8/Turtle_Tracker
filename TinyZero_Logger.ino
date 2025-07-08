#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include "BMA250.h"

const int chipSelect = 10;
File dataLogFile;
File startupLogFile;
File errorLogFile;

BMA250 accel;
RTC_DS1307 rtc;

unsigned long lastLogTime = 0;
const unsigned long interval = 10000;  // 10 seconds

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
  }
}

// Utility to write a timestamp string into dateStr and timeStr buffers
void getTimestamp(char* dateStr, size_t dateLen, char* timeStr, size_t timeLen) {
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    snprintf(dateStr, dateLen, "%04d-%02d-%02d", now.year(), now.month(), now.day());
    snprintf(timeStr, timeLen, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  } else {
    // RTC not running, fallback to placeholder
    snprintf(dateStr, dateLen, "0000-00-00");
    snprintf(timeStr, timeLen, "00:00:00");
  }
}

// Log message to startup.log with timestamp
void logStartup(const char* message) {
  if (!startupLogFile) return;

  char dateStr[11], timeStr[9];
  getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));

  startupLogFile.print(dateStr);
  startupLogFile.print(",");
  startupLogFile.print(timeStr);
  startupLogFile.print(",STARTUP,");
  startupLogFile.println(message);
  startupLogFile.flush();
}

// Log message to errors.log with timestamp
void logError(const char* message) {
  if (!errorLogFile) return;

  char dateStr[11], timeStr[9];
  getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));

  errorLogFile.print(dateStr);
  errorLogFile.print(",");
  errorLogFile.print(timeStr);
  errorLogFile.print(",ERROR,");
  errorLogFile.println(message);
  errorLogFile.flush();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLED(3, 200);

  SerialUSB.begin(115200);
  delay(3000);

  SerialUSB.println("Starting TinyZero logger...");
  delay(5000);

  // Initialize Wire
  SerialUSB.println("Starting Wire.begin()...");
  Wire.begin();
  SerialUSB.println("Wire.begin() done.");
  blinkLED(1, 300);

  // Initialize RTC
  SerialUSB.println("Initializing RTC...");
  if (!rtc.begin()) {
    SerialUSB.println("RTC init failed!");
    blinkLED(5, 300);
    while (1);
  }
  SerialUSB.println("RTC init done.");
  blinkLED(2, 300);

  // Initialize SD
  SerialUSB.println("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    SerialUSB.println("SD init failed!");
    blinkLED(5, 300);
    while (1);
  }
  SerialUSB.println("SD init done.");
  blinkLED(3, 300);

  // Create /d directory if needed
  if (!SD.exists("/d")) {
    SerialUSB.println("Creating /d directory...");
    if (!SD.mkdir("/d")) {
      SerialUSB.println("Failed to create /d directory!");
      blinkLED(5, 300);
      while (1);
    }
    SerialUSB.println("/d directory created.");
    blinkLED(4, 300);
  } else {
    SerialUSB.println("/d directory exists.");
  }

  // Open startup log file (append mode)
  startupLogFile = SD.open("/d/startup.log", FILE_WRITE);
  if (!startupLogFile) {
    SerialUSB.println("Failed to open startup.log!");
    blinkLED(5, 300);
    while (1);
  }
  logStartup("Logger setup started");

  // Open error log file (append mode)
  errorLogFile = SD.open("/d/errors.log", FILE_WRITE);
  if (!errorLogFile) {
    SerialUSB.println("Failed to open errors.log!");
    blinkLED(5, 300);
    while (1);
  }

  logStartup("Wire initialized");
  logStartup("RTC initialized");
  logStartup("SD card initialized");

  // Open data log file with current date
  DateTime now = rtc.now();
  char dataFilename[20];
  snprintf(dataFilename, sizeof(dataFilename), "/d/log%02d%02d.csv", now.month(), now.day());

  bool newFile = !SD.exists(dataFilename);
  dataLogFile = SD.open(dataFilename, FILE_WRITE);
  if (!dataLogFile) {
    SerialUSB.println("Failed to open data log file!");
    logError("Failed to open data log file");
    blinkLED(5, 300);
    while (1);
  }

  if (newFile) {
    dataLogFile.println("Date,Time,X,Y,Z,Temp(C)");
    dataLogFile.flush();
  }
  logStartup("Data log file opened");

  delay(500);
  SerialUSB.println("Initializing accelerometer...");
  accel.begin(BMA250_range_2g, BMA250_update_time_64ms);
  SerialUSB.println("Accelerometer initialized.");
  blinkLED(3, 100);

  logStartup("Accelerometer initialized");
  logStartup("Setup complete, entering main loop.");
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastLogTime >= interval) {
    lastLogTime = currentTime;

    accel.read();
    int x = accel.X;
    int y = accel.Y;
    int z = accel.Z;
    double temp = accel.rawTemp * 0.5 + 24.0;

    if (x == -1 && y == -1 && z == -1) {
      SerialUSB.println("ERROR: BMA250 NOT DETECTED");
      logError("BMA250 NOT DETECTED");
      blinkLED(5, 200);
      while (1);
    }

    char dateStr[11];
    char timeStr[9];
    getTimestamp(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));

    // Serial output
    SerialUSB.print("Log: ");
    SerialUSB.print(dateStr);
    SerialUSB.print(" | ");
    SerialUSB.print(timeStr);
    SerialUSB.print(" | X=");
    SerialUSB.print(x);
    SerialUSB.print(" Y=");
    SerialUSB.print(y);
    SerialUSB.print(" Z=");
    SerialUSB.print(z);
    SerialUSB.print(" | Temp=");
    SerialUSB.println(temp, 2);

    // Write sensor data to data log CSV
    dataLogFile.print(dateStr);
    dataLogFile.print(",");
    dataLogFile.print(timeStr);
    dataLogFile.print(",");
    dataLogFile.print(x);
    dataLogFile.print(",");
    dataLogFile.print(y);
    dataLogFile.print(",");
    dataLogFile.print(z);
    dataLogFile.print(",");
    dataLogFile.print(temp, 2);
    dataLogFile.println();
    dataLogFile.flush();

    // Flash LED once on log
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
