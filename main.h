#include <stdint.h>
#include <Arduino.h>  // Add this line to include the necessary definitions for String

#define PINA 35
#define PINB 34
#define PINC 39
#define PIND 36
#define TEMP1 32
#define TEMP2 33
#define DSGINPUT 16
#define CELL1_BALANCE 4
#define CELL2_BALANCE 5
#define CELL3_BALANCE 18
#define CELL4_BALANCE 19
#define CHARGERIN 23
#define CHARGEROUT 27
#define DMOSFET 13
#define CMOSFET 14

#define COV 3.65
#define CUV 2.5
#define OVTEMP 80
#define LWTEMP -10

#define CHARGING 1
#define DISCHARGING 0

#define ON  0x0
#define OFF 0x1

void OverVoltageCheck(float cell[]);
void UnderVoltageCheck(float cell[]);
void OverTempCheck(float temp1);
void ChargeFET(bool cfet);
void DischargeFET(bool dfet);
void checkBattChargeOrDischargeStatus(float cell[],float temp1);
void balancingMode(uint8_t balswitchON, uint8_t balanceswitch[]);
void checkForUpdate();
bool isNewVersionAvailable(const String& newVersion);
void updateFirmware();

extern int FETSTATUS;
extern float Dtemp, Ctemp;

extern void readVoltages(float cell[], float *packVoltage);
extern void AllFETSOFF();
extern void AllFETSON();
extern void AllGpioOFF();
extern void temperatureCheck(float *temp1);
