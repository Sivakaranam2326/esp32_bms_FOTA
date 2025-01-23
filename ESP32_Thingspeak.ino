#include "esp_sleep.h"
#include "esp32Wifi.h"
#include "esp32Adc.h"
#include "esp32Bms.h"
#include "main.h"
#include <stdint.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <HTTPClient.h>

int FETSTATUS = 0;
bool chargeFlag = 0;
bool balanceStatus = 0;

#define NUM_READINGS 10

// Firmware Version (current version of the firmware)
const String currentFirmwareVersion = "1.0.2";  // Replace with the actual version of your firmware

// URL for firmware and version metadata
const char* firmwareURL = "https://sivakaranam2326.github.io/esp32_bms_FOTA/ESP32_Thingspeak.ino.bin";
const char* versionURL = "https://sivakaranam2326.github.io/esp32_bms_FOTA/version.txt";  // URL for version info

void GpioInit() {
  pinMode(DSGINPUT, INPUT);
  pinMode(CELL1_BALANCE, OUTPUT);
  pinMode(CELL2_BALANCE, OUTPUT);
  pinMode(CELL3_BALANCE, OUTPUT);
  pinMode(CELL4_BALANCE, OUTPUT);
  pinMode(CHARGERIN, OUTPUT); 
  pinMode(CHARGEROUT, OUTPUT);
  pinMode(DMOSFET, OUTPUT);
  pinMode(CMOSFET, OUTPUT);
}

void AllGpioOFF() {
  digitalWrite(CELL1_BALANCE, OFF);
  digitalWrite(CELL2_BALANCE, OFF);
  digitalWrite(CELL3_BALANCE, OFF);
  digitalWrite(CELL4_BALANCE, OFF);
  digitalWrite(CHARGERIN, OFF);
  digitalWrite(CHARGEROUT, OFF);
  digitalWrite(DMOSFET, OFF);
  digitalWrite(CMOSFET, OFF);
}

void readVoltages(float cell[], float *packVoltage) {
  uint16_t adcValue[4] = {0};
  uint32_t sumPINA = 0;
  uint32_t sumPINB = 0;
  uint32_t sumPINC = 0;
  uint32_t sumPIND = 0;

  // Take 10 readings for each pin and calculate the sum
  for (int i = 0; i < NUM_READINGS; i++) {
    sumPINA += analogRead(PINA);
    sumPINB += analogRead(PINB);
    sumPINC += analogRead(PINC);
    sumPIND += analogRead(PIND);
    delay(10);  // Small delay between readings to ensure stability
  }

  // Calculate the average for each pin
  adcValue[0] = sumPINA / NUM_READINGS;
  adcValue[1] = sumPINB / NUM_READINGS;
  adcValue[2] = sumPINC / NUM_READINGS;
  adcValue[3] = sumPIND / NUM_READINGS;
  float rawdata[4] = {0};
  //Serial.println(adcValue[0]);
  for (int i = 0; i < 4; i++) {
    rawdata[i] = adcValue[i] * (3.3 / 4095.0);
  }
  delay(10);
  readCells(rawdata, cell, packVoltage);
}

float readTemperature(uint16_t tempPin) {
  int analogValue = analogRead(tempPin);
  float resistance = 8000.0 * (4095.0/ (float)analogValue - 1.0);
  float temperature = resistance / 100000.0;
  temperature = log(temperature);
  temperature /= 3950.0;
  temperature += 1.0 / (25 + 273.15);
  temperature = 1.0/ temperature;
  temperature -= 273.15;
  return temperature;
}

void AllFETSON() {
  digitalWrite(DMOSFET, ON);
  digitalWrite(CMOSFET, ON);
  FETSTATUS = 2;
}

void AllFETSOFF() {
  digitalWrite(DMOSFET, OFF);
  digitalWrite(CMOSFET, OFF);
  FETSTATUS = 0;
}

void OverVoltageCheck(float cell[]) {
  if ((cell[0] >= COV) || (cell[1] >= COV) || (cell[2] >= COV) || (cell[3] >= COV)) {
    ChargeFET(OFF);
    AllFETSON();
  }
}

void UnderVoltageCheck(float cell[]) {
  if ((cell[0] <= CUV) || (cell[1] <= CUV) || (cell[2] <= CUV) || (cell[3] <= CUV)) {
    DischargeFET(OFF);
    ChargeFET(ON);
    chargeFlag = 0;
  }
}

void OverTempCheck(float temp1){
  if(temp1 >= OVTEMP){
    ChargeFET(OFF);
  }
}

void ChargeFET(bool cfet) {
  digitalWrite(CMOSFET, cfet);
  digitalWrite(CHARGERIN, cfet);
  digitalWrite(CHARGEROUT, cfet);
  FETSTATUS = !cfet;
}

void DischargeFET(bool dfet) {
  digitalWrite(DMOSFET, dfet);
  FETSTATUS = !dfet;
}

void checkBattChargeOrDischargeStatus(float cell[],float temp1) {
  bool CDFlag = digitalRead(DSGINPUT);
  if (CDFlag == 1) {
    float HighestVoltage = cell[findHighestVoltage(cell, 4)];
    float LowestVoltage = cell[findLowestVoltage(cell, 4)];
    if ((HighestVoltage > 2.5 && HighestVoltage <= 3.65) && (chargeFlag == 0)) {
       DischargeFET(OFF);
       ChargeFET(ON);
       OverTempCheck(temp1);
    }
    else{
      // ideal mode
      ChargeFET(OFF);
      DischargeFET(ON);
      OverVoltageCheck(cell);
      UnderVoltageCheck(cell);
      if(HighestVoltage <= 3.3)
      chargeFlag = 0;
      else
      chargeFlag = 1;
    }

  } else {
    ChargeFET(OFF);
    AllFETSON();
    chargeFlag = 0;
  }
}

void balancingMode(uint8_t balswitchON, uint8_t balanceswitch[]) {

  if(balswitchON == 1){
      digitalWrite(CELL1_BALANCE, balanceswitch[0] ? ON : OFF);
      digitalWrite(CELL2_BALANCE, balanceswitch[1] ? ON : OFF);
      digitalWrite(CELL3_BALANCE, balanceswitch[2] ? ON : OFF);
      digitalWrite(CELL4_BALANCE, balanceswitch[3] ? ON : OFF);
      balanceStatus = 1;
  }
  else{
      digitalWrite(CELL1_BALANCE, OFF);
      digitalWrite(CELL2_BALANCE, OFF);
      digitalWrite(CELL3_BALANCE, OFF);
      digitalWrite(CELL4_BALANCE, OFF);
      balanceStatus = 0;
  }


      
}

void temperatureCheck(float *temp1) {
  *temp1 = readTemperature(TEMP1);
  //*temp2 = readTemperature(TEMP2);
}

void checkForUpdate() {
  HTTPClient http;
  http.begin(versionURL);  // Start HTTP client to fetch version information

  int httpCode = http.GET();  // Send the HTTP GET request to the version URL
  if (httpCode == 200) {
    String newFirmwareVersion = http.getString();  // Get the version string from the server
    newFirmwareVersion.trim();  // Clean up the version string

    if (isNewVersionAvailable(newFirmwareVersion)) {
      Serial.println("New firmware version available. Updating...");
      updateFirmware();  // Proceed to firmware update if new version is found
    } else {
      Serial.println("Already on the latest version.");
      Serial.println(newFirmwareVersion);
    }
  } else {
    Serial.println("Failed to fetch the firmware version. Skipping update check.");
  }

  http.end();  // Close the HTTP connection
}

bool isNewVersionAvailable(const String& newVersion) {
  // Compare the current firmware version with the new version
  return newVersion > currentFirmwareVersion;
}

void updateFirmware() {
  HTTPClient http;
  http.begin(firmwareURL);  // Start HTTP client to fetch firmware binary

  t_httpUpdate_return ret = httpUpdate.update(http, firmwareURL);  // Start firmware update

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No Update Available");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update Successful!");
      // ESP32 will automatically reboot and apply the new firmware after this.
      break;
  }

  http.end();  // Close the HTTP connection
}

void setup() {
  Serial.begin(115200);
  GpioInit();
  AllGpioOFF();
  delay(10);
  wifiInit();
  delay(500);
  checkForUpdate();  // Call the check for update function
  delay(20);
}

void loop() {
  static uint8_t secCounter = 0;
  float cell[4] = {0};
  float packVoltage = 0.0;
  float temp1 = 0.0;

  readVoltages(cell, &packVoltage);
  delay(1);
  temperatureCheck(&temp1);
  delay(1);
  bms_monitoring_fun(cell, packVoltage,temp1);
  //Serial.println(temp1,temp2);

  secCounter++;
  if (secCounter % 5 == 0) {
    dataUpdateToCloud(cell, packVoltage, temp1,balanceStatus);
    Serial.println("Data updated to cloud");
  }
  else if(secCounter % 12 == 0){
    checkForUpdate();  // Call the check for update function
    secCounter = 0;
  }

  delay(5000);
}
