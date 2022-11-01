#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"
#include "GPOutput.hpp"
#include "Pins.hpp"
#define mPRINT_PIN_CHANGE

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
    bleGamepadConfig.setHatSwitchCount(3); // 1 by default

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
    outputs = new GPOutput*[30]{
        new SimpleButton(new DigitalIGPin(ucPins, PIN_R_GEAR), BTN_R_GEAR),
        new SimpleButton(new DigitalIGPin(ucPins, PIN_L_GEAR), BTN_L_GEAR),

        new SimpleButton(new DigitalIGPin(ucPins, PIN_R_B1), BTN_R_B1),
        new SimpleButton(new DigitalIGPin(ucPins, PIN_R_B2), BTN_R_B2),
        new SimpleButton(new DigitalIGPin(ucPins, PIN_R_B3), BTN_R_B3),

        new SimpleButton(new DigitalIGPin(ucPinsExt, PIN_L_B1), BTN_L_B1),
        new SimpleButton(new DigitalIGPin(ucPins, PIN_L_B2), BTN_L_B2),
        new SimpleButton(new DigitalIGPin(ucPins, PIN_L_B3), BTN_L_B3),

        new SimpleButton(new DigitalIGPin(ucPinsExt, PIN_TB_CLICK), BTN_TB_CLICK),

        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_L_HAT_CLICK), BTN_L_HAT_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_L_HAT_A), BTN_L_HAT_A),
        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_L_HAT_B), BTN_L_HAT_B),

        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_L_TW_CLICK), BTN_L_TW_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_LT_SEL_CLICK), BTN_LT_SEL_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins1, PIN_LB_SEL_CLICK), BTN_LB_SEL_CLICK),

        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_R_HAT_CLICK), BTN_R_HAT_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_R_HAT_A), BTN_R_HAT_A),
        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_R_HAT_B), BTN_R_HAT_B),

        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_R_TW_CLICK), BTN_R_TW_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_RT_SEL_CLICK), BTN_RT_SEL_CLICK),
        new SimpleButton(new DigitalIGPin(pcfPins2, PIN_RB_SEL_CLICK), BTN_RB_SEL_CLICK),
        
        new Hat(
            new DigitalIGPin(pcfPins1, PIN_L_HAT_UP), 
            new DigitalIGPin(pcfPins1, PIN_L_HAT_DOWN), 
            new DigitalIGPin(pcfPins1, PIN_L_HAT_LEFT), 
            new DigitalIGPin(pcfPins1, PIN_L_HAT_RIGHT),
            HAT_LT),

        new Hat(
            new DigitalIGPin(pcfPins2, PIN_R_HAT_UP), 
            new DigitalIGPin(pcfPins2, PIN_R_HAT_DOWN), 
            new DigitalIGPin(pcfPins2, PIN_R_HAT_LEFT), 
            new DigitalIGPin(pcfPins2, PIN_R_HAT_RIGHT),
            HAT_RT),

        new Hat(
            new DigitalPulse(new DigitalIGPin(ucPinsExt, PIN_TB_UP)), 
            new DigitalPulse(new DigitalIGPin(ucPinsExt, PIN_TB_DOWN)), 
            new DigitalPulse(new DigitalIGPin(ucPinsExt, PIN_TB_LEFT)), 
            new DigitalPulse(new DigitalIGPin(ucPinsExt, PIN_TB_RIGHT)),
            HAT_TB),
        
        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins1, PIN_L_TW_A),
                new DigitalIGPin(pcfPins1, PIN_L_TW_B)),
            BTN_L_TW_UP, 
            BTN_L_TW_DOWN),

        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins1, PIN_LT_SEL_A),
                new DigitalIGPin(pcfPins1, PIN_LT_SEL_B)),
            BTN_LT_SEL_UP, 
            BTN_LT_SEL_DOWN),

        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins1, PIN_LB_SEL_A),
                new DigitalIGPin(pcfPins1, PIN_LB_SEL_B)),
            BTN_LB_SEL_UP, 
            BTN_LB_SEL_DOWN),

        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins2, PIN_R_TW_A),
                new DigitalIGPin(pcfPins2, PIN_R_TW_B)),
            BTN_R_TW_UP, 
            BTN_R_TW_DOWN),

        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins2, PIN_RT_SEL_A),
                new DigitalIGPin(pcfPins2, PIN_RT_SEL_B)),
            BTN_RT_SEL_UP, 
            BTN_RT_SEL_DOWN),

        new RelativeButtonsFromLinear(
            new Encoder(
                new DigitalIGPin(pcfPins2, PIN_RB_SEL_A),
                new DigitalIGPin(pcfPins2, PIN_RB_SEL_B)),
            BTN_RB_SEL_UP, 
            BTN_RB_SEL_DOWN),

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

    for (int i = 0; i < 30; i++)
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