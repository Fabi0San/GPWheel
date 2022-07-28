#include <Arduino.h>

// DigitalInput
  // debounce
  // last state
  // GetState
  // filtered state
  // isRaising
  // isFalling

// GPIO DigitalPin(pin#)
  // GetState

// RAM DigitalRam(Address, mask)
  // GetState


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


class DigitalInput
{
  private:
    int state;
    ulong readTime;
    ulong debounceLimit;

    virtual int readState() = 0;

  public: 
    DigitalInput(ulong debounceLimit);
    bool Update();
    virtual int GetState();
};

class DigitalPin : public DigitalInput
{
  private:
    uint8_t pin;
    int readState() override;

  public:
    DigitalPin(uint8_t pin, ulong debounceLimit);
};

class Encoder
{
  private:
    long count;
    DigitalInput* inputA;
    DigitalInput* inputB;
    
  public:
    Encoder(DigitalInput* inputA, DigitalInput* inputB);
    bool Update();
    long GetState();
};


DigitalInput::DigitalInput(ulong debounceLimit)
  : debounceLimit(debounceLimit), state(0), readTime(millis())
{
}

bool DigitalInput::Update()
{
  int newState = this->readState();
  int now;

  if(newState != this->state
      && (
        this->debounceLimit == 0 
        || (((now = millis()) - this->readTime) > this->debounceLimit)))
  {
    this->state = newState;
    this->readTime = now;
    return true;
  }

  return false;
}

int DigitalInput::GetState()
{
  return this->state;
}

DigitalPin::DigitalPin(uint8_t pin, ulong debounceLimit) 
  : DigitalInput(debounceLimit), pin(pin)
{   
}

int DigitalPin::readState()
{
  return digitalRead(pin);
}

Encoder::Encoder(DigitalInput* inputA, DigitalInput* inputB)
  : inputA(inputA), inputB(inputB), count(0)
  {    
  }

bool Encoder::Update()
{
  bool aUpdated = this->inputA->Update();
  bool bUpdated = this->inputB->Update();
  if(aUpdated == bUpdated)
    return false;
  
  this->count = aUpdated ^ (this->inputA->GetState() == this->inputB->GetState())
    ? this->count + 1
    : this->count -1;
  
  return true;
}

long Encoder::GetState()
{
  return this->count;
}


DigitalPin dp = DigitalPin(4, 5);
Encoder* enc = nullptr;

void setup() {
  Serial.begin(250000);
  Serial.println("setup");

  pinMode(4, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);

  enc = new Encoder(new DigitalPin(18,5), new DigitalPin(19,5));
  //dp = DigitalPin(4, 5);
}

int lastState = 0;
int count = 0;
bool shouldLog = true;

void loop() {
  // put your main code here, to run repeatedly:
  int currentState = dp.GetState();
  if(currentState != lastState)
  {
    count++;
    lastState = currentState;
  }

  if(enc->Update())
  {
      Serial.println(enc->GetState());
  }

/*
  if((millis() % 1000) == 0)
  {
    if(shouldLog)
    {
      Serial.println(enc->GetState());
      shouldLog = false;
    }
  }
  else
    shouldLog = true;
    */

}