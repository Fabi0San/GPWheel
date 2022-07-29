#include <Arduino.h>



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

// simple interface for input
class Input
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

int Input::GetState()
{
  return this->state;
}

int Input::HasChanged()
{
    return this->changed; 
}

class DigitalInput : public Input
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

class LinearInput : public Input
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
    Input* source;

  public: 
    DigitalIGPin(Input* source, int pin, ulong debounceLimit);
    virtual void Update() override;
};

DigitalIGPin::DigitalIGPin(Input* source, int pin, ulong debounceLimit)
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
    Input* inputA;
    Input* inputB;
    
  public:
    Encoder(Input* inputA, Input* inputB);
    virtual void Update() override;
};

Encoder::Encoder(Input* inputA, Input* inputB)
  : inputA(inputA), inputB(inputB)
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
      ? + 1
      : -1;
  
  this->state += this->changed;
}


ESP32DigitalInputGroup* ucPins = nullptr;
Encoder* enc = nullptr;
DigitalIGPin* btn = nullptr;

void setup() {
  Serial.begin(250000);
  Serial.println("setup");

  pinMode(4, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);

  ucPins = new ESP32DigitalInputGroup(bit(4)|bit(18)|bit(19), bit(4));
  enc = new Encoder(new DigitalIGPin(ucPins, 18, 5), new DigitalIGPin(ucPins, 19, 5));
  btn = new DigitalIGPin(ucPins, 4, 5);
}

int lastState = 0;
int count = 0;
bool shouldLog = true;

void loop() {

  ucPins->Update();
  enc->Update();
  btn->Update();

  if(enc->HasChanged())
  {
      Serial.println(enc->GetState());
  }

  if(btn->Falling())
  {
      Serial.println("released");
  }

  if(btn->Raising())
  {
      Serial.println("Pressed");
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