#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"
#include "GPOutput.hpp"


// AnalogInput
  // last state
  // GetState
  // currentState

// AnalogPin(pin)
  // GetState

// AnalogEncoder(pin, pin)
  // currentState

//GPControl
  // refresh state

// GPButton (digitalInput, bttn#)

// GPAxis(analogInput, gp#)

// GPSlider(analogInput, gp#)

// GPHat(digitalInput[4], gp#)


ESP32DigitalInputGroup* ucPins = nullptr;
Encoder* enc = nullptr;
DigitalIGPin* btn = nullptr;
BleGamepad bleGamepad;
RelativeButtonsFromLinear* updown = nullptr;

void setup() {
  Serial.begin(250000);
  Serial.println("setup");

  pinMode(4, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);

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

  
  ucPins = new ESP32DigitalInputGroup(bit(4)|bit(18)|bit(19), bit(4));
  enc = new Encoder(new DigitalIGPin(ucPins, 18, 5), new DigitalIGPin(ucPins, 19, 5));
  btn = new DigitalIGPin(ucPins, 4, 5);
  updown = new RelativeButtonsFromLinear(enc, 2, 3, 4); 
}

int lastState = 0;
int count = 0;
bool shouldLog = true;

void loop() {

  ucPins->Update();
  updown->Update(&bleGamepad);
  btn->Update();

  if(enc->HasChanged())
  {
      Serial.println(enc->GetState());
      bleGamepad.setAxes(enc->GetState());
      //bleGamepad.sendReport();
  }

  if(btn->Falling())
  {
    bleGamepad.release(BUTTON_1);
  }

  if(btn->Raising())
  {
    bleGamepad.press(BUTTON_1);
  }

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