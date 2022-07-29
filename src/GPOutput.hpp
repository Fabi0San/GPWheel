#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"

class RelativeButtonsFromLinear
{
  private:
    int buttonUp;
    int buttonDown;
    int buttonHeld;
    int divisor;
    LinearInput* source;
    ulong releaseAt;
    BleGamepad* bleGamepad;
  
  public:
    RelativeButtonsFromLinear(BleGamepad* bleGamepad, LinearInput* source, int buttonUp, int buttonDown, int divisor);
    void Update();
};

RelativeButtonsFromLinear::RelativeButtonsFromLinear(BleGamepad* bleGamepad, LinearInput* source, int buttonUp, int buttonDown, int divisor)
  : source(source), bleGamepad(bleGamepad), buttonUp(buttonUp), buttonDown(buttonDown), divisor(divisor), releaseAt(0), buttonHeld(0)
{
}

void RelativeButtonsFromLinear::Update()
{
  if(this->releaseAt !=0 && millis() >= this->releaseAt)
  {
    this->bleGamepad->release(this->buttonHeld);
    this->releaseAt = 0;
  }

  this->source->Update();

  if(this->releaseAt == 0
    && this->source->HasChanged() 
    && ((this->source->GetState() % this->divisor) == 0))
    {
      int button = this->source->Raising()
        ? this->buttonUp
        : this->buttonDown;
      this->bleGamepad->press(button);
      this->buttonHeld = button;
      this->releaseAt = millis() + 100;
    }
}
