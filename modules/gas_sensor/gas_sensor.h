//=====[#include guards - begin]===============================================

#ifndef _GAS_SENSOR_H_
#define _GAS_SENSOR_H_
#include "mbed.h"

//=====[Declaration of public defines]=========================================

//=====[Declaration of public data types]======================================

#include "fire_alarm.h"

float GasSen0127V();
float GasSenRead();
bool gasDetectedRead();

void gasSensorTickersISR();
//bool gasSensorRead();

//=====[#include guards - end]=================================================

#endif // _GAS_SENSOR_H_
