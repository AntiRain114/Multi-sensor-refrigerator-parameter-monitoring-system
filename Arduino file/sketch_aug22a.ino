#include <Wire.h>
#include <CayenneLPP.h>
#include "ClosedCube_HDC1080.h"
#include <MKRWAN.h>
#include <sam.h>

#define TCA9548A_ADDRESS 0x70
#define HDC1080_ADDRESS 0x40
#define LIGHT_SENSOR_PIN A1

ClosedCube_HDC1080 hdc1080_1;
ClosedCube_HDC1080 hdc1080_2;
ClosedCube_HDC1080 hdc1080_3;

LoRaModem modem;
CayenneLPP lpp(51);

const unsigned long SEND_INTERVAL = 10 * 60 * 1000; // 10 minutes in milliseconds
unsigned long lastSendTime = 0;

unsigned long lightCycleStartTime = 0;
unsigned long lightCycleDuration = 0;
bool isLightOn = false;

const int LIGHT_THRESHOLD = 750;

double tempChangeDuringCycle1 = 0, tempChangeDuringCycle2 = 0, tempChangeDuringCycle3 = 0;
double humChangeDuringCycle = 0;

double cycleStartTemp1, cycleStartTemp2, cycleStartTemp3, cycleStartHum;

String appEui = "0000000000000000";
String appKey = "";

// Watchdog variables
const unsigned long WATCHDOG_TIMEOUT = 12 * 60 * 1000; // 12 minutes in milliseconds
unsigned long lastWatchdogReset = 0;
unsigned long watchdogCounter = 0;

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  
  initializeSensor(hdc1080_1, 0);
  initializeSensor(hdc1080_2, 1);
  initializeSensor(hdc1080_3, 2);

  pinMode(LIGHT_SENSOR_PIN, INPUT);

  initLoRaConnection();

  resetAllVariables();

  lastWatchdogReset = millis();
}

void initializeSensor(ClosedCube_HDC1080 &sensor, int channel) {
  tcaselect(channel);
  sensor.begin(HDC1080_ADDRESS);
  
  // Configure HDC1080
  Wire.beginTransmission(HDC1080_ADDRESS);
  Wire.write(0x02);
  Wire.write(0x90);
  Wire.write(0x00);
  Wire.endTransmission();
  
  delay(20);  // Delay for startup of HDC1080
}

void loop() {
  unsigned long currentTime = millis();

  // 在每个循环开始时读取数据
  readSensorsForCycleStart();

  // Software watchdog check
  if (timeDiff(currentTime, lastWatchdogReset) >= 60000) {
    watchdogCounter++;
    if (watchdogCounter >= 12) {
      Serial.println("Watchdog timeout. Resetting...");
      resetSystem();
    }
    lastWatchdogReset = currentTime;
  }

  checkLightState(currentTime);

  if (timeDiff(currentTime, lastSendTime) >= SEND_INTERVAL) {
    if (checkAndReconnectLoRa()) {
      readSensorsAndSendData();
      lastSendTime = currentTime;
      resetAllVariables();
      watchdogCounter = 0;
    }
  }

  delay(1000);
}

void initLoRaConnection() {
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  modem.setADR(true);
  modem.dataRate(5);

  Serial.println("Connected to the network!");
}

bool checkAndReconnectLoRa() {
  if (!modem.connected()) {
    Serial.println("LoRaWAN disconnected. Attempting to reconnect...");
    if (!modem.joinOTAA(appEui, appKey)) {
      Serial.println("Reconnection failed");
      return false;
    }
    Serial.println("Reconnected to LoRaWAN");
  }
  return true;
}

void checkLightState(unsigned long currentTime) {
  int lightLevel = analogRead(LIGHT_SENSOR_PIN);
  
  if (lightLevel > LIGHT_THRESHOLD && !isLightOn) {
    isLightOn = true;
    lightCycleStartTime = currentTime;
  } else if (lightLevel <= LIGHT_THRESHOLD && isLightOn) {
    isLightOn = false;
    lightCycleDuration = timeDiff(currentTime, lightCycleStartTime);
    calculateEnvironmentChanges();
  }
}

void calculateEnvironmentChanges() {
  double currentTemp1, currentHum1, currentTemp2, currentTemp3;

  readSensor(hdc1080_1, 0, &currentTemp1, &currentHum1);
  readSensor(hdc1080_2, 1, &currentTemp2, NULL);
  readSensor(hdc1080_3, 2, &currentTemp3, NULL);

  // 使用默认值替换无效读数
  if (currentTemp1 == -999) currentTemp1 = 6.0;
  if (currentTemp2 == -999) currentTemp2 = 7.0;
  if (currentTemp3 == -999) currentTemp3 = 1.0;
  if (currentHum1 == -999) currentHum1 = 60.0;

  // 计算变化量
  tempChangeDuringCycle1 = currentTemp1 - cycleStartTemp1;
  tempChangeDuringCycle2 = currentTemp2 - cycleStartTemp2;
  tempChangeDuringCycle3 = currentTemp3 - cycleStartTemp3;
  humChangeDuringCycle = currentHum1 - cycleStartHum;

  Serial.print("Light cycle duration: ");
  Serial.print(lightCycleDuration);
  Serial.println(" ms");
  Serial.print("Temp change during cycle (sensor 1): ");
  Serial.println(tempChangeDuringCycle1);
  Serial.print("Temp change during cycle (sensor 2): ");
  Serial.println(tempChangeDuringCycle2);
  Serial.print("Temp change during cycle (sensor 3): ");
  Serial.println(tempChangeDuringCycle3);
  Serial.print("Humidity change during cycle: ");
  Serial.println(humChangeDuringCycle);
}

void readSensorsForCycleStart() {
  readSensor(hdc1080_1, 0, &cycleStartTemp1, &cycleStartHum);
  readSensor(hdc1080_2, 1, &cycleStartTemp2, NULL);
  readSensor(hdc1080_3, 2, &cycleStartTemp3, NULL);

  // 使用默认值替换无效读数
  if (cycleStartTemp1 == -999) cycleStartTemp1 = 6.0;
  if (cycleStartTemp2 == -999) cycleStartTemp2 = 7.0;
  if (cycleStartTemp3 == -999) cycleStartTemp3 = 1.0;
  if (cycleStartHum == -999) cycleStartHum = 60.0;
}

void readSensorsAndSendData() {
  double temp1, hum1, temp2, temp3;

  readSensor(hdc1080_1, 0, &temp1, &hum1);
  readSensor(hdc1080_2, 1, &temp2, NULL);
  readSensor(hdc1080_3, 2, &temp3, NULL);

  // 使用默认值替换无效读数
  if (temp1 == -999) temp1 = 6.0;
  if (temp2 == -999) temp2 = 7.0;
  if (temp3 == -999) temp3 = 1.0;
  if (hum1 == -999) hum1 = 60.0;

  unsigned long lightCycleSeconds = lightCycleDuration / 1000;

  lpp.reset();
  lpp.addTemperature(1, temp1);
  lpp.addRelativeHumidity(2, hum1);
  lpp.addTemperature(3, temp2);
  lpp.addTemperature(4, temp3);
  lpp.addDigitalInput(5, lightCycleSeconds);
  lpp.addTemperature(6, tempChangeDuringCycle1);
  lpp.addTemperature(7, tempChangeDuringCycle2);
  lpp.addTemperature(8, tempChangeDuringCycle3);
  lpp.addRelativeHumidity(9, humChangeDuringCycle);

  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  int err = modem.endPacket(true);

  if (err > 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }
}

void readSensor(ClosedCube_HDC1080 &sensor, int channel, double *temperature, double *humidity) {
  tcaselect(channel);
  
  uint8_t Byte[4];
  uint16_t temp, hum;

  Wire.beginTransmission(HDC1080_ADDRESS);
  Wire.write(0x00);  // Point to temperature register
  Wire.endTransmission();
  
  delay(20);  // Allow for conversion time
  
  Wire.requestFrom(HDC1080_ADDRESS, 4);
  
  if (4 <= Wire.available()) {
    Byte[0] = Wire.read();
    Byte[1] = Wire.read();
    Byte[2] = Wire.read();
    Byte[3] = Wire.read();

    temp = (((unsigned int)Byte[0] << 8) | Byte[1]);
    hum = (((unsigned int)Byte[2] << 8) | Byte[3]);

    double readTemp = (double)(temp) / 65536 * 165 - 40;
    double readHum = (double)(hum) / 65536 * 100;

    // Check if the temperature is within a reasonable range
    if (temperature) {
      if (readTemp >= -5 && readTemp <= 20) {
        *temperature = readTemp;
      } else {
        *temperature = -999; // Mark as invalid
      }
    }

    // Check if the humidity is within a reasonable range
    if (humidity) {
      if (readHum >= 30 && readHum <= 100) {
        *humidity = readHum;
      } else {
        *humidity = -999; // Mark as invalid
      }
    }
  } else {
    if (temperature) *temperature = -999;
    if (humidity) *humidity = -999;
    Serial.print("Error reading sensor on channel ");
    Serial.println(channel);
  }
}

void resetAllVariables() {
  lightCycleStartTime = 0;
  lightCycleDuration = 0;
  isLightOn = false;

  tempChangeDuringCycle1 = 0;
  tempChangeDuringCycle2 = 0;
  tempChangeDuringCycle3 = 0;
  humChangeDuringCycle = 0;

  cycleStartTemp1 = 6.0;
  cycleStartTemp2 = 7.0;
  cycleStartTemp3 = 1.0;
  cycleStartHum = 60.0;

  watchdogCounter = 0;

  Serial.println("All variables have been reset.");
}

unsigned long timeDiff(unsigned long current, unsigned long previous) {
  if (current >= previous) {
    return current - previous;
  } else {
    return (0xFFFFFFFF - previous) + current + 1;
  }
}

void resetSystem() {
  Serial.println("Resetting system...");
  Serial.flush(); // Ensure all serial data is sent
  
  // Trigger watchdog reset
  NVIC_SystemReset();
}