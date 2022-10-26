#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"
#include "GPOutput.hpp"
#include "Pins.hpp"
#define PRINT_PIN_CHANGE

ESP32DigitalInputGroup* ucPins = nullptr;
ESP32DigitalInputGroup* ucPinsExt = nullptr;
BleGamepad bleGamepad;
RelativeButtonsFromLinear* updown = nullptr;
SimpleButton* btn = nullptr;
Axis* axis = nullptr;
Hat* hat = nullptr;
GPOutput** outputs = nullptr;
PCF8575* pcf1 = nullptr;
PCF8575* pcf2 = nullptr;
PCFDigitalInputGroup* pcfPins1 = nullptr;
PCFDigitalInputGroup* pcfPins2 = nullptr;

void setup() 
{
    // serial setup
    Serial.begin(250000);
    Serial.println("setup");

    //PCF setup
    pcf1 = new PCF8575(0x20);
    pcf2 = new PCF8575(0x27);
    for (uint8_t i = 0; i < 16; i++)
    {
        pcf1->pinMode(i, INPUT);
        pcf2->pinMode(i, INPUT);
    }

    pcf1->begin();
    pcf2->begin();

    pcfPins1 = new PCFDigitalInputGroup(pcf1, 0xffff, 0x0);
    pcfPins2 = new PCFDigitalInputGroup(pcf2, 0xffff, 0x0);
    
    // PINS setup
    pinMode(32+PIN_L_B1, INPUT_PULLUP); 
    pinMode(PIN_L_B2, INPUT_PULLUP);
    pinMode(PIN_L_B3, INPUT_PULLUP);
    pinMode(PIN_R_B1, INPUT_PULLUP);
    pinMode(PIN_R_B2, INPUT_PULLUP);
    pinMode(PIN_R_B3, INPUT_PULLUP);

    pinMode(PIN_L_GEAR, INPUT); 
    pinMode(PIN_R_GEAR, INPUT); 
    
    pinMode(32+PIN_TB_CLICK, INPUT);
    pinMode(32+PIN_TB_DOWN, INPUT); 
    pinMode(32+PIN_TB_LEFT, INPUT); 
    pinMode(32+PIN_TB_RIGHT, INPUT);
    pinMode(32+PIN_TB_UP, INPUT);

    // Gamepad setup
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(true);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setButtonCount(64);
    bleGamepadConfig.setIncludeHome(true);
    bleGamepadConfig.setIncludeStart(true);
    bleGamepadConfig.setIncludeSelect(true);
    bleGamepadConfig.setWhichAxes(true, true, true, true, true, true, true, true);      // Can also be done per-axis individually. All are true by default
    //bleGamepadConfig.setWhichSimulationControls(true, true, true, true, true); // Can also be done per-control individually. All are false by default
    bleGamepadConfig.setHatSwitchCount(4);                                                                      // 1 by default

    bleGamepad.begin(&bleGamepadConfig);

    
    // INPUT setup
    ucPins = new ESP32DigitalInputGroup(
        GPIO_IN_REG, 
        bit(PIN_L_B2)|bit(PIN_L_B3)|bit(PIN_R_B1)|bit(PIN_R_B2)|
            bit(PIN_R_B3)|bit(PIN_R_GEAR)|bit(PIN_L_GEAR), // Inputs
        bit(PIN_L_B2)|bit(PIN_L_B3)|bit(PIN_R_B1)|bit(PIN_R_B2)|
            bit(PIN_R_B3)); // Flip

    ucPinsExt = new ESP32DigitalInputGroup(
        GPIO_IN1_REG, 
        bit(PIN_L_B1)|bit(PIN_TB_CLICK)|bit(PIN_TB_DOWN)|
            bit(PIN_TB_LEFT)|bit(PIN_TB_RIGHT)|bit(PIN_TB_UP), // inputs
        bit(PIN_L_B1)|bit(PIN_TB_CLICK)); // flip

    // OUTPUT SETUP
    outputs = new GPOutput*[13]{
        new SimpleButton(new DigitalIGPin(ucPinsExt, 1, 5), 1),
        new SimpleButton(new DigitalIGPin(ucPins, 25, 5), 2),
        new SimpleButton(new DigitalIGPin(ucPins, 26, 5), 3),
        new SimpleButton(new DigitalIGPin(ucPins, 27, 5), 4),
        new SimpleButton(new DigitalIGPin(ucPins, 14, 5), 5),
        new SimpleButton(new DigitalIGPin(ucPins, 12, 5), 6),
        new SimpleButton(new DigitalIGPin(ucPins, 5, 5), 7),
        new SimpleButton(new DigitalIGPin(ucPins, 23, 5), 8),
        new SimpleButton(new DigitalIGPin(ucPinsExt, 4, 5), 9),
        new SimpleButton(new DigitalIGPin(ucPinsExt, 7, 5), 10),
        new SimpleButton(new DigitalIGPin(ucPinsExt, 2, 5), 11),
        new SimpleButton(new DigitalIGPin(ucPinsExt, 3, 5), 12),
        new SimpleButton(new DigitalIGPin(ucPinsExt, 0, 5), 13)
    };

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

    /*hat = new Hat(
            new DigitalPulse(new DigitalIGPin(ucPins, 19, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 22, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 18, 5)), 
            new DigitalPulse(new DigitalIGPin(ucPins, 21, 5)), 
            1);*/

}

int lastState = 0;
int count = 0;
bool shouldLog = true;

void loop() {
    ucPins->Update();
    ucPinsExt->Update();
    pcfPins1->Update();
    pcfPins2->Update();
    //pcfPins1->Update();
    //updown->Update(&bleGamepad);
    //axis->Update(&bleGamepad);
    //btn->Update(&bleGamepad);
    //btn4->Update(&bleGamepad);
    //hat->Update(&bleGamepad);

    for (int i = 0; i < 13; i++)
    {
        outputs[i]->Update(&bleGamepad);
    }

#ifdef PRINT_PIN_CHANGE
    if(ucPins->HasChanged())
        Serial.printf("u1-%f\n", log(ucPins->HasChanged())/log(2));
    if(ucPinsExt->HasChanged())
        Serial.printf("u2-%f\n", log(ucPinsExt->HasChanged())/log(2));
    if(pcfPins1->HasChanged())
        Serial.printf("s1-%f\n", log(pcfPins1->HasChanged())/log(2));
    if(pcfPins2->HasChanged())
        Serial.printf("s2-%f\n", log(pcfPins2->HasChanged())/log(2));
#endif

/*
    //if((millis() % 1000) == 0)
    if(ucPins->HasChanged() || ucPinsExt->HasChanged() || pcfPins1->HasChanged() || pcfPins2->HasChanged())
    {
      //  if(shouldLog)
        {
            shouldLog = false;
        }
    }
    else
      shouldLog = true;*/
}