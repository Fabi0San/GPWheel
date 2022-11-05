#pragma once
#include <Arduino.h>
#include <BleGamepad.h>
#include "GPInput.hpp"

class GPOutput
{
    public:
    virtual int Update(BleGamepad* bleGamepad) = 0;
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
        int Update(BleGamepad* bleGamepad) override;
};

RelativeButtonsFromLinear::RelativeButtonsFromLinear(LinearInput* source, uint8_t buttonUp, uint8_t buttonDown, int divisor = 4)
    : source(source), buttonUp(buttonUp), buttonDown(buttonDown), divisor(divisor), releaseAt(0), buttonHeld(0)
{
}

int RelativeButtonsFromLinear::Update(BleGamepad* bleGamepad)
{
    int result = 0;
    if(this->releaseAt !=0 && millis() >= this->releaseAt)
    {
        bleGamepad->release(this->buttonHeld);
        this->releaseAt = 0;
        result++;
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
            result++;
        }
    
    return result;
}

class SimpleButton : public GPOutput
{
    private:
        uint8_t button;
        DigitalInput* source;
    public:
        SimpleButton(DigitalInput* source, uint8_t button);
        int Update(BleGamepad* bleGamepad) override;
};

SimpleButton::SimpleButton(DigitalInput* source, uint8_t button)
    : source(source), button(button)
{    
}

int SimpleButton::Update(BleGamepad* bleGamepad)
{
    this->source->Update();
    if(source->Raising())
    {
        bleGamepad->press(this->button);
        return 1;
    }

    if(source->Falling())
    {
        bleGamepad->release(this->button);
        return 1;
    }
}

class Axis : public GPOutput
{
    private:
        uint8_t axis;
        LinearInput* source;
    public:
        Axis(LinearInput* source, uint8_t axis);
        int Update(BleGamepad* bleGamepad) override;
};

Axis::Axis(LinearInput* source, uint8_t axis)
    : source(source), axis(axis)
{    
}

int Axis::Update(BleGamepad* bleGamepad)
{
    this->source->Update();

    if(source->HasChanged())
    {
        int16_t value = this->source->GetState();

        switch (this->axis)
        {
        case X_AXIS:
            bleGamepad->setX(value);
            break;
        case Y_AXIS:
            bleGamepad->setY(value);
            break;
        case Z_AXIS:
            bleGamepad->setZ(value);
            break;
        case RX_AXIS:
            bleGamepad->setRX(value);
            break;
        case RY_AXIS:
            bleGamepad->setRY(value);
            break;
        case RZ_AXIS:
            bleGamepad->setRZ(value);
            break;
        case SLIDER1:
            bleGamepad->setSlider1(value);
            break;
        case SLIDER2:
            bleGamepad->setSlider2(value);
            break;       
        }
        return 1;
    }
    return 0;
}

class Hat: public GPOutput
{
    private:
        uint8_t hat;
        DigitalInput* upSrc; 
        DigitalInput* downSrc;
        DigitalInput* leftSrc; 
        DigitalInput* rightSrc;

        static signed char HatFromUDLR(byte udlr);

    public:
        Hat(DigitalInput* upSrc, DigitalInput* downSrc, DigitalInput* leftSrc, DigitalInput* rightSrc, uint8_t hat);
        int Update(BleGamepad* bleGamepad) override;
};

Hat::Hat(DigitalInput* upSrc, DigitalInput* downSrc, DigitalInput* leftSrc, DigitalInput* rightSrc, uint8_t hat)
    : upSrc(upSrc), downSrc(downSrc), leftSrc(leftSrc), rightSrc(rightSrc), hat(hat) {}

signed char Hat::HatFromUDLR(byte urdl)
{
    switch (urdl)
    {
        case bit(0): return HAT_UP;
        case bit(1): return HAT_RIGHT;
        case bit(2): return HAT_DOWN;
        case bit(3): return HAT_LEFT;
        case bit(0)|bit(1): return HAT_UP_RIGHT;
        case bit(0)|bit(3): return HAT_UP_LEFT;
        case bit(2)|bit(1): return HAT_DOWN_RIGHT;
        case bit(2)|bit(3): return HAT_DOWN_LEFT;
        default: return HAT_CENTERED;
    }
}

int Hat::Update(BleGamepad* bleGamepad)
{
    this->upSrc->Update();
    this->rightSrc->Update();
    this->downSrc->Update();
    this->leftSrc->Update();

    if(this->upSrc->HasChanged() ||
        this->rightSrc->HasChanged() ||
        this->downSrc->HasChanged() ||
        this->leftSrc->HasChanged())
    {
        byte urdl = 
            (this->upSrc->GetState() ? bit(0) : 0) |
            (this->rightSrc->GetState() ? bit(1) : 0) |
            (this->downSrc->GetState() ? bit(2) : 0) |
            (this->leftSrc->GetState() ? bit(3) : 0);

        signed char newState = Hat::HatFromUDLR(urdl);

        switch (this->hat)
        {
            case 1:
                bleGamepad->setHat1(newState);
                break;
            case 2:
                bleGamepad->setHat2(newState);
                break;
            case 3:
                bleGamepad->setHat3(newState);
                break;
            case 4:
                bleGamepad->setHat4(newState);
                break;
        }
        return 1;
    }
    return 0;
}