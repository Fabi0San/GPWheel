#pragma once
#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"

class GPOutput
{
    public:
    virtual void Update(BleGamepad* bleGamepad) = 0;
};

class RelativeButtonsFromLinear : public GPOutput
{
    private:
        uint8_t buttonUp;
        uint8_t buttonDown;
        uint8_t buttonHeld;
        int divisor;
        LinearInput* source;
        ulong releaseAt;
  
    public:
        RelativeButtonsFromLinear(LinearInput* source, uint8_t buttonUp, uint8_t buttonDown, int divisor);
        void Update(BleGamepad* bleGamepad) override;
};

RelativeButtonsFromLinear::RelativeButtonsFromLinear(LinearInput* source, uint8_t buttonUp, uint8_t buttonDown, int divisor)
    : source(source), buttonUp(buttonUp), buttonDown(buttonDown), divisor(divisor), releaseAt(0), buttonHeld(0)
{
}

void RelativeButtonsFromLinear::Update(BleGamepad* bleGamepad)
{
    if(this->releaseAt !=0 && millis() >= this->releaseAt)
    {
        bleGamepad->release(this->buttonHeld);
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
        bleGamepad->press(button);
        this->buttonHeld = button;
        this->releaseAt = millis() + 100;
        }
}

class SimpleButton : public GPOutput
{
    private:
        uint8_t button;
        DigitalInput* source;
    public:
        SimpleButton(DigitalInput* source, uint8_t button);
        void Update(BleGamepad* bleGamepad) override;
};

SimpleButton::SimpleButton(DigitalInput* source, uint8_t button)
    : source(source), button(button)
{    
}

void SimpleButton::Update(BleGamepad* bleGamepad)
{
    this->source->Update();
    if(source->Raising())
    {
        bleGamepad->press(this->button);
    }

    if(source->Falling())
    {
        bleGamepad->release(this->button);
    }
}