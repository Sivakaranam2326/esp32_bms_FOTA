#include "esp32Bms.h"
#include "main.h"
#include "esp32Adc.h"

#define POV 14.6
#define PUV 10

BATTERY_STATUS checkBattStatus(float packVoltage) {
  if (packVoltage >= POV) {
    return BATTERYOVERVOLTAGE;
  } else if (packVoltage <= PUV) {
    return BATTERYUNDERVOLTAGE;
  } else {
    return BATTERYNORMALSTATE;
  }
}

BATTERY_STATUS checkBattChargeOrDischarge(float cell[],float temp1) {
  checkBattChargeOrDischargeStatus(cell,temp1);
  if (FETSTATUS == 0) {
    return BATTERYOPENCIRCUITMODE;
  }else if(FETSTATUS == 1){
    return BATTERYCHARGING;
  } else {
    return BATTERYDISCHARGING;
  }
}

BATTERY_STATUS checkBalancing(float cell[], uint8_t balswitch[]) {
  float LowestVoltage = cell[findLowestVoltage(cell, 4)];
  float difference = 0.0;
  for (int i = 0; i < 4; i++) {
    difference = cell[i] - LowestVoltage;
    if ((difference >= 0.100) && (cell[i] >= 3.55)) {
      balswitch[i] = 1;
    }
    else{
      balswitch[i] = 0;
    }
  }
  if((balswitch[0] == 1 || balswitch[1] == 1) || (balswitch[2] == 1 || balswitch[3] == 1))
  return BATTERYBALANCINGON;
  else
  return BATTERYBALANCINGOFF;
}

void bms_monitoring_fun(float cell[], float packVoltage, float temp1) {
  uint8_t balswitch[4] = {0};
  BATTERY_STATUS status = checkBattStatus(packVoltage);

  if (status == BATTERYOVERVOLTAGE){
    ChargeFET(OFF);
    AllFETSON();
  }
  else if(status == BATTERYUNDERVOLTAGE) {
    DischargeFET(OFF);
    ChargeFET(ON);
  }
  else {
    BATTERY_STATUS chargestatus = checkBattChargeOrDischarge(cell,temp1);
    if(chargestatus == BATTERYCHARGING){
      BATTERY_STATUS balanceStatus = checkBalancing(cell,balswitch);
      if (balanceStatus == BATTERYBALANCINGON) {
        balancingMode(1,balswitch);
      }
      else if (balanceStatus == BATTERYBALANCINGOFF) {
        balancingMode(0,balswitch);
      }
    }
    else{
      // balance is not required
      balancingMode(0,balswitch);
    }
  }
}
