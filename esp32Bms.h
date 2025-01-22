#include <stdint.h>

typedef enum {
  BATTERYCHARGING = 1,
  BATTERYDISCHARGING,
  BATTERYOPENCIRCUITMODE,
  BATTERYNORMALSTATE,
  BATTERYOVERVOLTAGE,
  BATTERYUNDERVOLTAGE,
  BATTERYBALANCINGON,
  BATTERYBALANCINGOFF
} BATTERY_STATUS;

void bms_monitoring_fun(float cell[], float packVoltage, float temp1);
BATTERY_STATUS checkBattStatus(float packVoltage);
BATTERY_STATUS checkBattChargeOrDischarge(float cell[],float temp1);
BATTERY_STATUS checkBalancing(float cell[], uint8_t balswitch[]);
