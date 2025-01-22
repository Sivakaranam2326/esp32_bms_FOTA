#include "esp32Adc.h"
#include <Arduino.h>

void readCells(float rawcell[], float cell[], float *packVoltage) {
  rawcell[0] *= 2.23;
  rawcell[1] *= 4.85;
  rawcell[2] *= 6.36;
  rawcell[3] *= 8.63;
  if (rawcell[3] != 0.0) rawcell[3] -= rawcell[2];
  if (rawcell[2] != 0.0) rawcell[2] -= rawcell[1];
  if (rawcell[1] != 0.0) rawcell[1] -= rawcell[0];
  for (int i = 0; i < 4; i++) {
    cell[i] = rawcell[i];
  }
  *packVoltage = cell[0] + cell[1] + cell[2] + cell[3];
}

int findHighestVoltage(float cell[], int size) {
  int index = 0;
  for (int i = 1; i < size; i++) {
    if (cell[i] > cell[index]) {
      index = i;
    }
  }
  return index;
}

int findLowestVoltage(float cell[], int size) {
  int index = 0;
  for (int i = 1; i < size; i++) {
    if (cell[i] < cell[index]) {
      index = i;
    }
  }
  return index;
}
