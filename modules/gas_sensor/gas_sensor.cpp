//=====[Libraries]=============================================================

#include "mbed.h"

#include "gas_sensor.h"




AnalogIn GasSen0127(A2);


float Gas_Level            = 0.0;



float GasSen0127V(float analogRead)
{

    return (analogRead*4800);
}
float GasSenRead (Gas_Level){
    return Gas_Level;
}



void gasSensorInit()
{
}

void gasSensorUpdate()
{Gas_Level = GasSen0127V(GasSen0127);
}



//=====[Implementations of private functions]=================================