#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"
#include "GPOutput.hpp"

ESP32DigitalInputGroup* ucPins = nullptr;
BleGamepad bleGamepad;
RelativeButtonsFromLinear* updown = nullptr;
SimpleButton* btn = nullptr;
SimpleButton* btn4 = nullptr;
Axis* axis = nullptr;
Hat* hat = nullptr;

void setup() 
{
    Serial.begin(250000);
    Serial.println("setup");

    pinMode(4, INPUT_PULLUP);
    pinMode(18, INPUT_PULLDOWN);
    pinMode(19, INPUT_PULLDOWN);
    pinMode(21, INPUT_PULLDOWN);
    pinMode(22, INPUT_PULLDOWN);

    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    pinMode(27, INPUT_PULLUP);
    pinMode(26, INPUT_PULLUP);


    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(true);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setButtonCount(32);
    bleGamepadConfig.setIncludeHome(true);
    bleGamepadConfig.setIncludeStart(true);
    bleGamepadConfig.setIncludeSelect(true);
    bleGamepadConfig.setWhichAxes(true, true, true, true, true, true, true, true);      // Can also be done per-axis individually. All are true by default
    //bleGamepadConfig.setWhichSimulationControls(true, true, true, true, true); // Can also be done per-control individually. All are false by default
    bleGamepadConfig.setHatSwitchCount(4);                                                                      // 1 by default

    bleGamepad.begin(&bleGamepadConfig);

    
    ucPins = new ESP32DigitalInputGroup(bit(4)|bit(18)|bit(19)|bit(21)|bit(22)| bit(13)|bit(14)|bit(27)|bit(26), bit(4) | bit(13)|bit(14)|bit(27)|bit(26));
    btn = new SimpleButton(new DigitalIGPin(ucPins, 4, 5), 1);
    btn4 = new SimpleButton(new DigitalIGPin(ucPins, 21, 5), 4);
    /*updown = new RelativeButtonsFromLinear(
        new Encoder(
            new DigitalIGPin(ucPins, 18, 5), 
            new DigitalIGPin(ucPins, 19, 5),
            1, -12, 12)
        , 2, 3, 4); */

/*    axis = new Axis(
        new DirectionalPulse(
            new DigitalIGPin(ucPins, 18, 5), 
            new DigitalIGPin(ucPins, 19, 5),
            1000, -32000, 32000)
        , X_AXIS); */

    /*axis = new Axis(
        new ADCPin(13)
        , X_AXIS);  */

/*    hat = new Hat(
            new DigitalIGPin(ucPins, 13, 5), 
            new DigitalIGPin(ucPins, 14, 5), 
            new DigitalIGPin(ucPins, 27, 5), 
            new DigitalIGPin(ucPins, 26, 5),
            1);*/

    hat = new Hat(
            new DigitalPulse(new DigitalIGPin(ucPins, 19, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 22, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 18, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 21, 5)), 
            1);

}

int lastState = 0;
int count = 0;
bool shouldLog = true;

void loop() {
    ucPins->Update();
    //updown->Update(&bleGamepad);
    //axis->Update(&bleGamepad);
    btn->Update(&bleGamepad);
    //btn4->Update(&bleGamepad);
    hat->Update(&bleGamepad);

    if((millis() % 1000) == 0)
    {
        if(shouldLog)
        {
            Serial.print(".");
            shouldLog = false;
        }
    }
    else
      shouldLog = true;
}