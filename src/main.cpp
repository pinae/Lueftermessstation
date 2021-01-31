#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <sdios.h>
#include "sdHelpers.h"

#define SWITCH_PIN 22
#define VOLTAGE_PWM_CHANNEL 0
#define PIN_4_PWM_CHANNEL 1
#define VOLTAGE_PWM_PIN 0
#define PIN_4_PWM_PIN 32
#define TACHO_PIN 34
#define SD_CS_PIN 5
#define SD_SCK_PIN 18
#define SD_MISO_PIN 19
#define SD_MOSI_PIN 23
#define SPI_SPEED SD_SCK_MHZ(4)
#define HISTORY_SIZE 16

unsigned long lastTrigger;
SPIClass spi = SPIClass(VSPI);
SdFat sd;
//SdFs sd;
//typedef FsFile file_t;
//ofstream file;
ArduinoOutStream cout(Serial);
//File file;
bool regulateVoltage = false;
char buf[80];
unsigned long durationHistory[HISTORY_SIZE];
unsigned int usedHistory = 0;
unsigned int historyPos = HISTORY_SIZE + 1;

ICACHE_RAM_ATTR void onFanRotation() {
  unsigned long now = millis();
  if (historyPos < HISTORY_SIZE) {
    unsigned long rotationDuration = now - lastTrigger;
    durationHistory[historyPos] = rotationDuration;
    historyPos++;
    if (usedHistory < HISTORY_SIZE) usedHistory = historyPos;
    historyPos = historyPos % HISTORY_SIZE;
  } else {
    historyPos = 0;
  }
  lastTrigger = now;
}

float calculateAverageRPM() {
  if (usedHistory <= 0) return -1.0;
  if (millis() - lastTrigger > 2000) { //reset if there is no rotation signal for 2s
    durationHistory[0] = durationHistory[historyPos - 1];
    historyPos = 0;
    usedHistory = 1;
    return 0.0;
  }
  float sum = 0.0;
  unsigned int usedValues = 0;
  for (unsigned int i = 0; i < usedHistory; i++) {
    sum += durationHistory[i % HISTORY_SIZE];
    usedValues++;
  }
  float averageDurationInMillis = sum / float(usedValues);
  float rpm = 1000 * 60 / averageDurationInMillis;
  return rpm;
}

void setup() {
  Serial.begin(115200);
  ledcSetup(VOLTAGE_PWM_CHANNEL, 1000, 8);
  ledcAttachPin(VOLTAGE_PWM_PIN, VOLTAGE_PWM_CHANNEL);
  ledcSetup(PIN_4_PWM_CHANNEL, 25000, 8);
  ledcAttachPin(PIN_4_PWM_PIN, PIN_4_PWM_CHANNEL);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(TACHO_PIN, INPUT_PULLUP);
  pinMode(SD_CS_PIN, OUTPUT);
  //spi.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  while(!Serial) {yield();}  // Wait for Serial to start
  printSpiPins();
  if (!sd.begin(SD_CS_PIN, SPI_SPEED)) {
    if (sd.card()->errorCode()) {
      printSdInitError(int(sd.card()->errorCode()), int(sd.card()->errorData()));
      return;
    }
    if (sd.vol()->fatType() == 0) {
      cout << F("Can't find a valid FAT16/FAT32 partition.\n");
      reformatMsg();
      return;
    }
    cout << F("begin failed, can't determine error type\n");
    return;
  }
  cout << F("\nCard successfully initialized.\n");
  cout << endl;
  printCardSizeInfo(sd);
  if (!sd.exists("data.csv")) {
    ofstream file("data.csv", ios::out | ios::app);
    file << "Voltage [%], PWM Signal [%], RPM" << endl;
    file.close();
  }
  lastTrigger = millis();
  attachInterrupt(digitalPinToInterrupt(TACHO_PIN), onFanRotation, RISING);
}

void loop() {
  long now = millis();
  long phase = abs((now/5000) % (20 * 2) - 20);
  Serial.println(phase);
  regulateVoltage = (digitalRead(SWITCH_PIN) == 0);
  obufstream bout(buf, sizeof(buf));
  if (regulateVoltage) {
    ledcWrite(VOLTAGE_PWM_CHANNEL, map(phase, 0, 20, 0, 255));
    bout << (phase * 5);
  } else {
    ledcWrite(VOLTAGE_PWM_CHANNEL, 255);
    bout << 100;
  }
  ledcWrite(PIN_4_PWM_CHANNEL, map(phase, 0, 20, 0, 255));
  bout << ", " << (phase * 5);
  bout << ", " << calculateAverageRPM();
  bout << endl;
  cout << buf;
  ofstream file("data.csv", ios::out | ios::app);
  file << buf;
  if (!file) {
    sd.errorHalt("append failed");
  }
  file.close();
}