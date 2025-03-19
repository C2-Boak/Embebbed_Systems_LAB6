//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"


#include "user_interface.h"

#include "code.h"
#include "siren.h"
#include "smart_home_system.h"
#include "fire_alarm.h"
#include "date_and_time.h"
#include "temperature_sensor.h"
#include "gas_sensor.h"
#include "matrix_keypad.h"
#include "display.h"

//=====[Declaration of private defines]========================================

#define DISPLAY_REFRESH_TIME_MS 1000
#define Message_Duration_MS 5000
#define Message_Display_Interval_MS 500



static Timer flashTimer;
static Timer displayTimer;

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

char codeSequenceFromUserInterface[CODE_NUMBER_OF_KEYS];

//=====[Declaration and initialization of private global variables]============

static bool incorrectCodeState = OFF;
static bool systemBlockedState = OFF;
static bool displayGasState = OFF;
static bool displayTempState = OFF;
static bool display_TOK = OFF;
static bool display_GOK = OFF;
static bool GasAlarmTrigger;
static bool TempAlarmTrigger;
static bool flashMessageOn = false;
static bool CodeEntry = false;

static bool codeComplete = false;
static int numberOfCodeChars = 0;
static Timer alarmAlertTimer;

//=====[Declarations (prototypes) of private functions]========================

static void userInterfaceMatrixKeypadUpdate();
static void incorrectCodeIndicatorUpdate();
static void systemBlockedIndicatorUpdate();

static void userInterfaceDisplayInit();
static void userInterfaceDisplayUpdate();
static void alarmActivation();
static void AlarmTriggerAlert();


//=====[Implementations of public functions]===================================

void userInterfaceInit()
{
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
    matrixKeypadInit( SYSTEM_TIME_INCREMENT_MS );
    userInterfaceDisplayInit();

    flashTimer.start();
    displayTimer.start();
}

void userInterfaceUpdate()
{
    userInterfaceMatrixKeypadUpdate();
    incorrectCodeIndicatorUpdate();
    systemBlockedIndicatorUpdate();
    userInterfaceDisplayUpdate();
}

bool incorrectCodeStateRead()
{
    return incorrectCodeState;
}

void incorrectCodeStateWrite( bool state )
{
    incorrectCodeState = state;
}

bool systemBlockedStateRead()
{
    return systemBlockedState;
}

void systemBlockedStateWrite( bool state )
{
    systemBlockedState = state;
}

bool userInterfaceCodeCompleteRead()
{
    return codeComplete;
}

void userInterfaceCodeCompleteWrite( bool state )
{
    codeComplete = state;
}
void alarmActivation(){

    GasAlarmTrigger = gasDetectedRead();
    TempAlarmTrigger = Temp_Range();

    if (GasAlarmTrigger || TempAlarmTrigger) {
        AlarmTriggerAlert();


    }
}

//=====[Implementations of private functions]==================================

static void userInterfaceMatrixKeypadUpdate()
{

    static int numberOfHashKeyReleased = 0;
    char keyReleased = matrixKeypadUpdate();

    if (keyReleased != '\0') {

        if (keyReleased == '2') {
            display_GOK = true;
            display_TOK = false;
            flashTimer.reset();
            flashMessageOn = true;

        } else if (keyReleased == '3') {
            display_GOK = false;
            display_TOK = true;

            flashTimer.reset();
            flashMessageOn = true;
        }

        if (sirenStateRead() && !systemBlockedStateRead()) {
            alarmActivation();
            if (!incorrectCodeStateRead()) {
                codeSequenceFromUserInterface[numberOfCodeChars] = keyReleased;
                numberOfCodeChars++;
                if (numberOfCodeChars >= CODE_NUMBER_OF_KEYS) {
                    codeComplete = true;
                    numberOfCodeChars = 0;
                }
            } else {
                if (keyReleased == '#') {
                    numberOfHashKeyReleased++;
                    if (numberOfHashKeyReleased >= 2) {
                        numberOfHashKeyReleased = 0;
                        numberOfCodeChars = 0;
                        codeComplete = false;
                        incorrectCodeState = OFF;
                    }
                }
            }


            if (keyReleased == '2' && GasAlarmTrigger){
                displayGasState = true;
                displayTempState = false;
                flashTimer.reset();
                flashMessageOn = true;
            }else {
                display_GOK = true;
            }
            if (keyReleased == '3' && TempAlarmTrigger){
                displayTempState = true;
                displayGasState = false;
                flashTimer.reset();
                flashMessageOn = true;
            }else{
                display_TOK = true;
            }
        }
    }
}

static void userInterfaceDisplayInit()
{
    displayInit(DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER);

    displayCharPositionWrite(0, 0);
    displayStringWrite("Temperature:");

    displayCharPositionWrite(0, 1);
    displayStringWrite("Gas:");

    displayCharPositionWrite(0, 2);
    displayStringWrite("Alarm:");
}

static void userInterfaceDisplayUpdate()
{
    static int accumulatedDisplayTime = 0;
    char temperatureString[3] = "";
    char gasString[4] = "";
    char stateString[27] = "";

    if (accumulatedDisplayTime >= DISPLAY_REFRESH_TIME_MS) {

        accumulatedDisplayTime = 0;

        alarmActivation();


        sprintf(temperatureString, "%.0f", temperatureSensorReadCelsius());
        displayCharPositionWrite(12, 0);
        displayStringWrite(temperatureString);
        displayCharPositionWrite(14, 0);
        displayStringWrite("'C");


        sprintf(gasString, "%.0f", GasSenRead());
        displayCharPositionWrite(4, 1);
        displayStringWrite(gasString);
        displayCharPositionWrite(8, 1);
        displayStringWrite("PPM");


        displayCharPositionWrite(6, 2);
        if (sirenStateRead()) {
            displayStringWrite("ON ");
        } else {
            displayStringWrite("OFF");
        }


        if (flashMessageOn) {
            if (flashTimer.read_ms() < Message_Duration_MS) {
                // Flash message at set intervals
                if (flashTimer.read_ms() % Message_Display_Interval_MS < (Message_Display_Interval_MS / 2)) {
                    if (displayGasState) {
                        sprintf(stateString, "Gas Level high");
                        displayCharPositionWrite(0, 3);
                        displayStringWrite(stateString);
                    } else if (displayTempState) {
                        sprintf(stateString, "High Temp detected");
                        displayCharPositionWrite(0, 3);
                        displayStringWrite(stateString);

                    }else if (display_TOK) {
                        sprintf(stateString, "Temp OK");
                        displayCharPositionWrite(0, 3);
                        displayStringWrite(stateString);
                    }else if (display_GOK) {
                        sprintf(stateString, "Gas Level Ok");
                        displayCharPositionWrite(0, 3);
                        displayStringWrite(stateString);
                    } else {

                        displayCharPositionWrite(0, 3);
                        displayStringWrite("                    ");
                    }
                } else {

                    display_GOK = false;
                    display_TOK = false;
                    displayGasState = false;
                    displayTempState = false;
                    flashMessageOn = false;

                    displayCharPositionWrite(0, 3);
                    displayStringWrite("                   ");
                }
            }
        }




    } else {
        accumulatedDisplayTime += SYSTEM_TIME_INCREMENT_MS;
    }
}




static void AlarmTriggerAlert() {

    int i;

    for (i = 0; i <10; i++) {

        if (GasAlarmTrigger || TempAlarmTrigger) {
            if (alarmAlertTimer.read() == 0.0) {
                alarmAlertTimer.start();
            }


            if (alarmAlertTimer.read() > 1.0) {
                static bool toggle = false;


                displayCharPositionWrite(0, 3);
                displayStringWrite("                    ");


                if (toggle) {

                    displayCharPositionWrite(0, 3);
                    displayStringWrite("Enter code to reset!");
                } else {

                    if (GasAlarmTrigger) {
                        displayCharPositionWrite(0, 3);
                        displayStringWrite("Gas alarm trigger!");
                    }
                    if (TempAlarmTrigger) {
                        displayCharPositionWrite(0, 3);
                        displayStringWrite("Temp alarm trigger!");
                    }
                }

                toggle = !toggle;
                delay(1000);

                alarmAlertTimer.reset();


            }
        }
    }


    if (!GasAlarmTrigger || !TempAlarmTrigger){
        displayCharPositionWrite(0, 3);
        displayStringWrite("                    ");
    }
}



static void incorrectCodeIndicatorUpdate()

{
    incorrectCodeLed = incorrectCodeStateRead();
}

static void systemBlockedIndicatorUpdate()
{
    systemBlockedLed = systemBlockedState;
}