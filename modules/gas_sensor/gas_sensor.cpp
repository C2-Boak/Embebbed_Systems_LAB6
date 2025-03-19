#include "fire_alarm.h"
#include "mbed.h"
#include "gas_sensor.h"


#define Dangerous_Gas_Level                    2000


AnalogIn GasSen0127(A2);


float Gas_Level            = 0.0;

Ticker gasSensorTicker;

float GasSen0127V(float analogRead)
{

    return (analogRead*4800);

}

float GasSenRead (){
    Gas_Level = GasSen0127V(GasSen0127);

    return Gas_Level;

}

bool gasDetectedRead(){
    if (Gas_Level > Dangerous_Gas_Level) {
        return 1 ;
    }else{
        return 0;
    }
}
// ISR function to read the gas sensor at regular intervals
void gasSensorTickerISR() {
    GasSenRead();  // Read the gas sensor value
}

void startGasSensorReading() {
    // Set the ticker to call `gasSensorTickerISR` every second (1.0 sec)
    gasSensorTicker.attach(&gasSensorTickerISR, 0.2);
}

void stopGasSensorReading() {
    // Stop the ticker if needed
    gasSensorTicker.detach();
}

