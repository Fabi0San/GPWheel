#pragma once
#include <Arduino.h>

// simple interface for input
class GPInput
{
  protected:
    int state = 0;
    int changed = 0;

  public: 
    int HasChanged();
    int GetState();  
    virtual void Update() = 0;
    virtual int Raising() = 0;  
    virtual int Falling() = 0;  
};

int GPInput::GetState()
{
  return this->state;
}

int GPInput::HasChanged()
{
    return this->changed; 
}

class DigitalInput : public GPInput
{
  public: 
    int Raising() override;  
    int Falling() override;  
};


int DigitalInput::Raising()
{
    return this->changed & this->state; 
}

int DigitalInput::Falling()
{
    return this->changed & ~this->state; 
}

class LinearInput : public GPInput
{
  public: 
    int Raising() override;  
    int Falling() override;  
};

int LinearInput::Raising()
{
    return this->changed > 0; 
}

int LinearInput::Falling()
{
    return this->changed < 0; 
}

// reads all ESP32 pins at once.
class ESP32DigitalInputGroup : public DigitalInput
{ 
  private:
    int mask;
    int flip;
  public:
      ESP32DigitalInputGroup(int mask, int flip);
      virtual void Update() override;
};

ESP32DigitalInputGroup::ESP32DigitalInputGroup(int mask, int flip) 
  : mask(mask), flip(flip)
{}

void ESP32DigitalInputGroup::Update()
{
  int newState = (REG_READ(GPIO_IN_REG) & this->mask) ^ this->flip;
  this->changed = newState ^ this->state;
  this->state = newState;
}

// Debounced pin from a group
class DigitalIGPin : public DigitalInput
{
  private:
    int pin;
    ulong readTime;
    ulong debounceLimit;
    GPInput* source;

  public: 
    DigitalIGPin(GPInput* source, int pin, ulong debounceLimit);
    virtual void Update() override;
};

DigitalIGPin::DigitalIGPin(GPInput* source, int pin, ulong debounceLimit)
  : source(source), pin(pin), debounceLimit(debounceLimit)
{    
}

void DigitalIGPin::Update()
{
  int newState = ((this->source->GetState() & bit(this->pin)) == 0) ? 0 : 1;
  ulong now;

  if(newState != this->state
      && (
        this->debounceLimit == 0 
        || (((now = millis()) - this->readTime) > this->debounceLimit)))
  {   
    this->changed = this->state ^ newState;
    this->state = newState;
    this->readTime = now;
  }
  else 
    this->changed = 0;
}

class Encoder : public LinearInput
{
  private:
    GPInput* inputA;
    GPInput* inputB;
    int magnification;
    int minValue;
    int maxValue;
    
  public:
    Encoder(GPInput* inputA, GPInput* inputB, int magnification, int minValue, int maxValue);
    virtual void Update() override;
};

Encoder::Encoder(GPInput* inputA, GPInput* inputB, int magnification, int minValue, int maxValue)
  : inputA(inputA), inputB(inputB), magnification(magnification), minValue(minValue), maxValue(maxValue)
  {    
  }

void Encoder::Update()
{
  this->inputA->Update();
  this->inputB->Update();

  bool aUpdated = this->inputA->HasChanged();
  bool bUpdated = this->inputB->HasChanged();

  this->changed = (aUpdated == bUpdated)
    ? 0
    : (aUpdated ^ (this->inputA->GetState() == this->inputB->GetState()))
      ? this->magnification
      : -this->magnification;

  this->state = max(min(this->state + this->changed , this->maxValue), this->minValue);
}
