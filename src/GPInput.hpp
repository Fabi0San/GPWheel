#pragma once
#include <Arduino.h>
#include "PCF8575.h"

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
    int reg;
    int mask;
    int flip;
  public:
      ESP32DigitalInputGroup(int reg, int mask, int flip);
      virtual void Update() override;
};

ESP32DigitalInputGroup::ESP32DigitalInputGroup(int reg,int mask, int flip) 
  : reg(reg), mask(mask), flip(flip)
{}

void ESP32DigitalInputGroup::Update()
{
  int newState = (REG_READ(this->reg) & this->mask) ^ this->flip;
  this->changed = newState ^ this->state;
  this->state = newState;
}



// reads values from i2c
class PCFDigitalInputGroup : public DigitalInput
{ 
  private:
    int mask;
    int flip;
    PCF8575* source;
  public:
      PCFDigitalInputGroup(PCF8575* source, int mask, int flip);
      virtual void Update() override;
};

PCFDigitalInputGroup::PCFDigitalInputGroup(PCF8575* source,int mask, int flip) 
  : source(source), mask(mask), flip(flip)
{}

void PCFDigitalInputGroup::Update()
{
  int newState = (this->source->digitalReadAll() & this->mask) ^ this->flip;
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

DigitalIGPin::DigitalIGPin(GPInput* source, int pin, ulong debounceLimit = 5)
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

class ADCPin : public LinearInput
{
    private:
        uint8_t pin;
        bool invert;
        bool autoExpand;
        uint16_t inputMin;
        uint16_t inputMax;
        int outputMin;
        int outputMax;
        uint16_t lastReading;
    
    public:
        ADCPin(uint8_t pin, bool invert, int inputMin, int inputMax, bool autoExpand, int outputMin, int outputMax);
        virtual void Update() override;
};

ADCPin::ADCPin(uint8_t pin, bool invert = false, int inputMin = 0, int inputMax = 4095, bool autoExpand = false, int outputMin = INT16_MIN, int outputMax = INT16_MAX)
    : pin(pin), invert(invert), autoExpand(autoExpand), inputMin(inputMin), inputMax(inputMax), outputMin(outputMin), outputMax(outputMax) {}

void ADCPin::Update()
{
    uint16_t reading = analogRead(pin);
    if(reading == this->lastReading)
        return;

    this->lastReading = reading;

    if(this->autoExpand)
    {
        this->inputMin = min(this->inputMin, reading);
        this->inputMax = min(this->inputMax, reading);
    }
   
    int newState = map(constrain(reading, this->inputMin, this->inputMax), this->inputMin, this->inputMax, this->outputMin, this->outputMax);

    this->changed = newState - this->state;
    this->state = newState;
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

Encoder::Encoder(GPInput* inputA, GPInput* inputB, int magnification = 1, int minValue = INT32_MIN, int maxValue = INT32_MAX)
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

  this->state = constrain(this->state + this->changed, this->minValue, this->maxValue);
}

class DirectionalPulse : public LinearInput
{
  private:
    GPInput* inputA;
    GPInput* inputB;
    int magnification;
    int minValue;
    int maxValue;
    
  public:
    DirectionalPulse(GPInput* inputA, GPInput* inputB, int magnification, int minValue, int maxValue);
    virtual void Update() override;
};

DirectionalPulse::DirectionalPulse(GPInput* inputA, GPInput* inputB, int magnification, int minValue, int maxValue)
  : inputA(inputA), inputB(inputB), magnification(magnification), minValue(minValue), maxValue(maxValue)
  {    
  }

void DirectionalPulse::Update()
{
    this->inputA->Update();
    this->inputB->Update();
    this->changed = 0;

    bool aUpdated = this->inputA->HasChanged();
    bool bUpdated = this->inputB->HasChanged();

    if(aUpdated ^ bUpdated)
    {
        this->changed = aUpdated ? magnification : -magnification;
        this->state = constrain(this->state + this->changed, this->minValue, this->maxValue);  
    }
}

// Helpful with track ball pulses
class DigitalPulse : public DigitalInput
{
  private:
    DigitalInput* source;
    ulong relaseTime;
  public:
    DigitalPulse(DigitalInput* source);
    virtual void Update() override;      
};

  DigitalPulse::DigitalPulse(DigitalInput* source)
    : source(source) {}

void DigitalPulse::Update()
{
  if(this->state)
  {
    if(this->relaseTime > millis())
      return;

    this->state = 0;
    this->changed = 1;
  }

  this->source->Update();
  
  if(this->source->HasChanged())
  {
    this->state = 1; //;this->state ? 0 : 1;
    this->changed = 1;
    this->relaseTime = millis() + 15;
  }
}