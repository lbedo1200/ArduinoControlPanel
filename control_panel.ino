#include <Keyboard.h>
const int Dispatch = 0;
int DispatchPOS = 0;
const int EStopButton = 1;
int EStopPOS = 0;
const int AcknowledgeButton = 2;
int ResetPOS = 0;
const int OPEnable = 3;
int OPEnablePOS = 0;
const int Restraints = 4;
int RestraintsPOS = 0;
const int Gates = 5;
int GatesPOS = 0;

const int AcknowledgeLight = 12;
const int EStopLight = 13;
const int Alarm = 11;

bool fault = false;
unsigned long resetTimerStart = 0;
unsigned long previousMillis = 0;
int lastRPOS = 0;
int lastGPOS = 0;


void setup()
{
  pinMode(Dispatch, INPUT_PULLUP);          //Button that dispatches train when safe
  pinMode(EStopButton, INPUT_PULLUP);       //Button that stops trains on the next available block and stop the lift
  pinMode(AcknowledgeButton, INPUT_PULLUP); //Button that acknowledges fault and clears it
  
  pinMode(OPEnable, INPUT_PULLUP);          //Operator enable switch that enables the dispatch button
  pinMode(Restraints, INPUT_PULLUP);        //Restraints switch that opens and closes restraints
  pinMode(Gates, INPUT_PULLUP);             //Entrance Gate switch that opens and closes gates
  
  pinMode(AcknowledgeLight, OUTPUT);        //Light on Acknowlegde button that turns on when button is pressed
  pinMode(EStopLight, OUTPUT);              //Light on E-Stop button that turns on when pressed and blinks when released
  pinMode(Alarm, OUTPUT);                   //Alarm sounder that beeps when system is faulted
  Keyboard.begin();
}

void loop()
{
  //Variables with the button positions (HIGH = Unpressed, LOW = Pressed)
  EStopPOS = digitalRead(EStopButton);
  DispatchPOS = digitalRead(Dispatch);
  ResetPOS = digitalRead(AcknowledgeButton);
  GatesPOS = digitalRead(Gates);
  RestraintsPOS = digitalRead(Restraints);
  OPEnablePOS = digitalRead(OPEnable);

  //Resets dispatch, Estop, and reset buttons. Could probably be done better
  ResetButtons();
  
  //EStop Button logic
  //If pushed, Light up button and set fault to true if not already
  //Else, Turn off light if no fault
  if(EStopPOS == LOW)
  {
    digitalWrite(EStopLight, HIGH);
    if (fault == false)
    {
      EStop();
    } 
  }
  else if(fault == false)
  {
    digitalWrite(EStopLight, LOW);
  }

  
  //Fault Light
  //Makes EStop light blink if eStop button is pulled out and there's a fault 
  if(EStopPOS == HIGH && fault == true)
      FaultLight();

  //Reset Button
  //Turns on Reset Light if pushed in, else turns off
  //calls Reset function if pressed and there's a fault
  if(ResetPOS == LOW)
  {
    digitalWrite(AcknowledgeLight, HIGH);
    if(fault == true)
      Reset();
  }
  else
  {
    digitalWrite(AcknowledgeLight, LOW);
    resetTimerStart = 0;
  }


  //Logic for Entrance Gate Switch
  //Presses 9 on keyboard if switched on
  //Presses 3 on keyboard if switched off
  if(GatesPOS != lastGPOS)
  {
    if(GatesPOS == LOW)
    {
      Keyboard.write(57);
      delay(100);
    }
    else if(GatesPOS == HIGH)
    {
      Keyboard.write(51);
      delay(100);
    }
  }
  lastGPOS = GatesPOS;

  //Restraint switch logic
  //Presses 8 on keyboard if switched on
  //Presses 2 on keyboard if switched off
  if(RestraintsPOS != lastRPOS)
  {
    if(RestraintsPOS == LOW)
    {
      Keyboard.write(56);
      delay(100);
    }
    else if(RestraintsPOS == HIGH)
    {
      Keyboard.write(50);
      delay(100);
    }
  }
  lastRPOS = RestraintsPOS;
  
  //Dispatch button logic
  //If button is pressed, there is no fault, and the restraints and gates are both closed, press enter on the keyboard
  if (DispatchPOS == LOW && fault != true && OPEnablePOS == LOW && GatesPOS == HIGH && RestraintsPOS == HIGH)
  {
    Keyboard.press(176);
  } 
}

//Presses and Holds F8 on keyboard and sets fault to true if called
void EStop()
{
  Keyboard.press(KEY_F8);
  fault = true;
}


//If acknowlege button is pressed for 2 seconds, set fault to false, and let go of F8
void Reset()
{
  if (resetTimerStart == 0)
  {
    resetTimerStart = millis();
  }
  digitalWrite(AcknowledgeLight, HIGH);
    
  if (millis() > resetTimerStart + 2000)
  { 
    fault = false;
    Keyboard.press(KEY_F8);
    resetTimerStart = 0;
  }
}

//Makes EStop button blink, but allows it to be interupted
void FaultLight()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500)
  {
    previousMillis = currentMillis;
    if (digitalRead(EStopLight) == LOW)
    {
      digitalWrite (Alarm, HIGH);
      digitalWrite(EStopLight, HIGH);
    }
    else
    {
      digitalWrite(Alarm, LOW);
      digitalWrite(EStopLight, LOW);
    }
  }
}


//Releases the enter key if button is let go
//Releases F8 key if estop and reset buttons aren't pressed
void ResetButtons()
{
  if (digitalRead(Dispatch) == HIGH)
    Keyboard.release(176);
  if (digitalRead(AcknowledgeButton) == HIGH && digitalRead(EStopButton) == HIGH)
    Keyboard.release(KEY_F8);
}
