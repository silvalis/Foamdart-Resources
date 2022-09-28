
/*
    Flycore, Brushless, Pusher
    for Seeed Xiao RP2040 
    Ori32 4 in 1 25A blheli_32
    Emax 1108 4600KV
    Pololu 4036 DRV8876 Motor Driver    
    N20 1000rpm
    Pololu 2843 5V 500mA VReg

    Input:
    Fire (on/off)
    Rev (on/off)
    Pusher Retracted (on/off)
    Select (on/off)

    Output:
    ESC1 (servo pwm)
    ESC2 (servo pwm)
    DRV EN (pwm %)
    DRV PH (high/low)

    author boont

    v0.1 Draft 02/09/2022


    Code Credits:
    Pico RP2040 library: Earle Philhower https://github.com/earlephilhower/arduino-pico
    Switch Debouncer - ISR debounce: Khoi Hoang https://github.com/khoih-prog/RPI_PICO_TimerInterrupt
    Hardware PWM: Khoi Hoang https://github.com/khoih-prog/RP2040_PWM

    TODO: 
    Do we want to use ISR Servo libraries? Probably not?
    How many of the pins require debouncing/interrupts? 
      Rev - yes?. Fire - no?. Pusher - Yes. Select - No. 
      Need to test program latencey for Rev.

*/



// These define's must be placed at the beginning before #include "TimerInterrupt_Generic.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         1
#define _TIMERINTERRUPT_LOGLEVEL_     4

// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "RPi_Pico_TimerInterrupt.h"

#include <Servo.h>
#include "RP2040_PWM.h" 

#include <Adafruit_NeoPixel.h>

/*
   Pin Definitions

   INPUTS
   -------
   D2 Fire
   D3 REV
   
   D7 SELECTOR
   D8 PUSHER LOCATION


   OUTPUTS
   -------
   DRV8876 - This is a 1.3A driver.
   note 12VDC N20 1000rpm, 600mA stalled, 280mA max efficiency, 190mA no load
   D9 PUSH EN       PWM
   D10 PUSH PH

   EN   PH  Op Mode
   0    X   Brake (shorted gnd)
   PWM  1   Foward PWM%
   PWM  0   Reverse PWM%

   ESC:
   D0 ESC1       Servo
   D1 ESC2        Servo

   MIS
   -------
   11 RGB LED
   25 LED

*/

    
// INPUTS
#define PIN_FIRE        D2
#define PIN_REV         D3

#define PIN_SELE        D7
#define PIN_PUSH        D8

// OUTPUTS
#define PIN_EN          D9      // PWM
#define PIN_PH          D10

#define PIN_ESC1        D0   // Servo
#define PIN_ESC2        D1    // Servo

// MISC
#define PIN_ULED        25
#define PIN_RGB_PWR     11
#define PIN_RGB         12
#define NUMPIXELS       1

//don't really need to reassign here, but considering for a generic function instead.
unsigned int SWPin = PIN_REV;
unsigned int SW2Pin = PIN_PUSH;

#define TIMER1_INTERVAL_MS        15
#define TIMER2_INTERVAL_MS        5    // note a 600rpm pusher = 1 rev every 100ms. debounce works by counting up  to timer/debounce (eg 20/80ms = 4 ticks). the pusher switch window is about 1/4 of a rotation, so its only on for 25ms.
#define DEBOUNCING_INTERVAL_MS    60
#define PUSHER_DEBOUNCING_INTERVAL_MS    20   // therefore i have set it to 5/20ms
#define LONG_PRESS_INTERVAL_MS    5000

#define LOCAL_DEBUG               2

#define BTN_LOW                   0
#define BTN_HIGH                  1
#define BTN_ROSE                  2
#define BTN_FELL                  3

// Motors Setup
#define MotorKV 2700
Servo MainMotor1;
Servo MainMotor2;
bool MotorsRunning = false;
int MinMotorSpeed = 1040;         // in pwm
int TargetMotorSpeed = 1960;      // in pwm freq
int CalcMotorSpeed = 1960;
int thirtypcMotorSpeed = 1316; // (1960-1040)/10*3

volatile bool SpinDown = false;
int SpinDownTime = 1500;    // 1.5 second spin down
int SpinDownStep = 100;
unsigned long lastspindownstep = 0;
int SpinDownRampRate = (TargetMotorSpeed-thirtypcMotorSpeed)/(SpinDownTime/SpinDownStep); // 92 counts per step
int SpinDownSteps[] = {13,21,34,55,55,55,55,55,55,55,55,55,55,55,55};   // maybe should auto generate this?
int SpinDown_i = 0;

// Neopixel setup
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_RGB , NEO_GRB + NEO_KHZ800);

// Init RPI_PICO_Timer, can use any from 0-15 pseudo-hardware timers
RPI_PICO_Timer ITimer1(1);
RPI_PICO_Timer ITimer2(2);

volatile bool SWPressed     = false;
volatile bool SWLongPressed = false;

volatile bool SW2Pressed     = false;
volatile bool SW2LongPressed = false;


volatile byte FireButtonState = BTN_HIGH;
volatile byte SelButtonState = BTN_HIGH;
volatile byte RevButtonState = BTN_HIGH;
volatile byte PushButtonState = BTN_HIGH;

bool PusherIsReset = false;



// Pusher setup
#define AUTO                        1
#define SEMI                        0

float freq = 20000.0f;
float dutyCycle_off = 0.0f;
float dutyCycle_full = 99.0f;
float dutyCycle_semi = 66.0f;
float dutyCycle_reset = 25.0f;
RP2040_PWM* PWM_Instance;



volatile int currentShot = 0;




// ISR Debounce Functions

// Rev Trigger

bool TimerHandler1(struct repeating_timer *t)
{ 
  static unsigned int debounceCountSWPressed  = 0;
  static unsigned int debounceCountSWReleased = 0;

#if (LOCAL_DEBUG > 1)
  static unsigned long SWPressedTime;
  static unsigned long SWReleasedTime;

  unsigned long currentMillis = millis();
#endif

  if ( (!digitalRead(SWPin)) )
  {
    // Start debouncing counting debounceCountSWPressed and clear debounceCountSWReleased
    debounceCountSWReleased = 0;

    if (++debounceCountSWPressed >= DEBOUNCING_INTERVAL_MS / TIMER1_INTERVAL_MS)
    {
      // Call and flag SWPressed
      if (!SWPressed)
      {
#if (LOCAL_DEBUG > 1)   
        SWPressedTime = currentMillis;
        
        Serial.print("SW Press, from millis() = "); Serial.println(SWPressedTime);
#endif

        SWPressed = true;

        // start - beacuse the switch is pressed by default
        StartMotors();
        SpinDown = false;
        SpinDown_i = 0;
        CalcMotorSpeed = TargetMotorSpeed;

        
        // Do something for SWPressed here in ISR
        // But it's better to use outside software timer to do your job instead of inside ISR
        //Your_Response_To_Press();
      }

//      if (debounceCountSWPressed >= LONG_PRESS_INTERVAL_MS / TIMER1_INTERVAL_MS)
//      {
//        // Call and flag SWLongPressed
//        if (!SWLongPressed)
//        {
//#if (LOCAL_DEBUG > 1)
//          Serial.print("SW Long Pressed, total time ms = "); Serial.print(currentMillis);
//          Serial.print(" - "); Serial.print(SWPressedTime);
//          Serial.print(" = "); Serial.println(currentMillis - SWPressedTime);                                           
//#endif
//
//          SWLongPressed = true;
//          // Do something for SWLongPressed here in ISR
//          // But it's better to use outside software timer to do your job instead of inside ISR
//          //Your_Response_To_Long_Press();
//        }
//      }
    }
  }
  else
  {
    // Start debouncing counting debounceCountSWReleased and clear debounceCountSWPressed
    if ( SWPressed && (++debounceCountSWReleased >= DEBOUNCING_INTERVAL_MS / TIMER1_INTERVAL_MS))
    {
#if (LOCAL_DEBUG > 1)      
      SWReleasedTime = currentMillis;

      // Call and flag SWPressed
      Serial.print("SW Released, from millis() = "); Serial.println(SWReleasedTime);
#endif

      SWPressed     = false;
//      SWLongPressed = false;

    // stop - switch is pressed by default
//      StopMotors();
SpinDown = true;

      // Do something for !SWPressed here in ISR
      // But it's better to use outside software timer to do your job instead of inside ISR
      //Your_Response_To_Release();

      // Call and flag SWPressed
#if (LOCAL_DEBUG > 1)
      Serial.print("SW Pressed total time ms = ");
      Serial.println(SWReleasedTime - SWPressedTime);
#endif

      debounceCountSWPressed = 0;
    }
  }

  return true;
}


//------------------------------------------------------------------------
// Pusher location
//------------------------------------------------------------------------
bool TimerHandler2(struct repeating_timer *t)
{ 
  static unsigned int debounceCountSW2Pressed  = 0;
  static unsigned int debounceCountSW2Released = 0;

#if (LOCAL_DEBUG > 1)
  static unsigned long SW2PressedTime;
  static unsigned long SW2ReleasedTime;

  unsigned long currentMillis = millis();
#endif

  if ( (!digitalRead(SW2Pin)) )
  {
    // Start debouncing counting debounceCountSWPressed and clear debounceCountSWReleased
    debounceCountSW2Released = 0;

    if (++debounceCountSW2Pressed >= PUSHER_DEBOUNCING_INTERVAL_MS / TIMER2_INTERVAL_MS)
    {
      // Call and flag SWPressed
      if (!SW2Pressed)
      {
#if (LOCAL_DEBUG > 1)   
        SW2PressedTime = currentMillis;
        
        Serial.print("SW2 Press, from millis() = "); Serial.println(SW2PressedTime);
#endif

        SW2Pressed = true;

//        if(( SelButtonState == SEMI ) || ( SelButtonState == AUTO && digitalRead( PIN_FIRE ) == BTN_LOW ))  // semi, or auto and not firing
   //     {
   //       StopPusher();
   //       PushButtonState = BTN_HIGH;
  //      }
  



  if( digitalRead( PIN_FIRE ) == BTN_LOW ) //&& !PusherIsReset )
      {
        Serial.println("Resetting Pusher");
        ResetPusher();
      }
      PushButtonState = BTN_LOW;
        
        
        // Do something for SWPressed here in ISR
        // But it's better to use outside software timer to do your job instead of inside ISR
        //Your_Response_To_Press();
      }

//      if (debounceCountSW2Pressed >= LONG_PRESS_INTERVAL_MS / TIMER1_INTERVAL_MS)
//      {
//        // Call and flag SWLongPressed
//        if (!SW2LongPressed)
//        {
//#if (LOCAL_DEBUG > 1)
//          Serial.print("SW2 Long Pressed, total time ms = "); Serial.print(currentMillis);
//          Serial.print(" - "); Serial.print(SW2PressedTime);
//          Serial.print(" = "); Serial.println(currentMillis - SW2PressedTime);                                           
//#endif
//
//          SW2LongPressed = true;
//          // Do something for SWLongPressed here in ISR
//          // But it's better to use outside software timer to do your job instead of inside ISR
//          //Your_Response_To_Long_Press();
//        }
//      }
    }
  }
  else
  {
    // Start debouncing counting debounceCountSWReleased and clear debounceCountSWPressed
    if ( SW2Pressed && (++debounceCountSW2Released >= PUSHER_DEBOUNCING_INTERVAL_MS / TIMER2_INTERVAL_MS))
    {
#if (LOCAL_DEBUG > 1)      
      SW2ReleasedTime = currentMillis;

      // Call and flag SWPressed
      Serial.print("SW2 Released, from millis() = "); Serial.println(SW2ReleasedTime);
#endif
 //     PusherIsReset = false;
      SW2Pressed     = false;
      SW2LongPressed = false;

  if(( SelButtonState == SEMI ) || ( SelButtonState == AUTO && digitalRead( PIN_FIRE ) == BTN_LOW ))  // semi, or auto and not firing
  {
    Serial.println("Stopping Pusher");
    StopPusher();
    PushButtonState = BTN_HIGH;
 //   PusherIsReset = true;
  }
    
      // Do something for !SWPressed here in ISR
      // But it's better to use outside software timer to do your job instead of inside ISR
      //Your_Response_To_Release();

      // Call and flag SWPressed
#if (LOCAL_DEBUG > 1)
      Serial.print("SW2 Pressed total time ms = ");
      Serial.println(SW2ReleasedTime - SW2PressedTime);
#endif

      debounceCountSW2Pressed = 0;
    }
  }

  return true;
}






void StartMotors()
{
  // SET_ESC_FULL_ON;
  Serial.println("Start ESC requested");
  MotorsRunning = true;
  MainMotor1.writeMicroseconds( TargetMotorSpeed );
  MainMotor2.writeMicroseconds( TargetMotorSpeed );

}

void StopMotors()
{
  //  SET_ESC_FULL_OFF;

  Serial.println("Stop ESC requested");
  MotorsRunning = false;
  MainMotor1.writeMicroseconds( MinMotorSpeed );
  MainMotor2.writeMicroseconds( MinMotorSpeed );
//  StopMillis = millis();

}

void StartPusher()
{
//Serial.print(F("Change PWM DutyCycle to ")); Serial.println(dutyCycle_full);
  if( SelButtonState == AUTO )
  {
    PWM_Instance->setPWM(PIN_EN, freq, dutyCycle_full, true);
  }
  else
  {
    PWM_Instance->setPWM(PIN_EN, freq, dutyCycle_semi, true);
  }
}

void StopPusher()
{
//Serial.print(F("Change PWM DutyCycle to ")); Serial.println(dutyCycle_off);
PWM_Instance->setPWM(PIN_EN, freq, dutyCycle_off, true);
}

void ResetPusher()
{
  //Serial.print(F("Resetting ")); Serial.println(dutyCycle_off);
PWM_Instance->setPWM(PIN_EN, freq, dutyCycle_reset, true);
}



void ProcessSelector()
{
  if( digitalRead( PIN_SELE ) )
  {
    
    SelButtonState = AUTO;
   //     Serial.print((millis() ));Serial.print(F(" Select  ")); Serial.println(SelButtonState);
  }
  else
  {
    SelButtonState = SEMI;
  //      Serial.print((millis() ));Serial.print(F(" Select "));  Serial.println(SelButtonState);
  }
    
}






void ProcessFire()
{

//Serial.println( currentShot );  
  if( digitalRead( PIN_FIRE ) && SWPressed )
  {
    if( SelButtonState == AUTO || (  SelButtonState == SEMI && currentShot == 0 ) )
    {
      StartPusher();
      PushButtonState = BTN_LOW;  
      FireButtonState = BTN_HIGH;
      currentShot++;
    }
    FireButtonState = BTN_LOW;
  }
 // else if (FireButtonState == BTN_HIGH && digitalRead( PIN_FIRE ) == LOW)
 else
  {
    FireButtonState = BTN_LOW;
    //  StopPusher();
    currentShot = 0;

//    Serial.println( currentShot );  
  }


}




void ProcessMotors()
{
  if( SpinDown )
  {
    unsigned long currenttime = millis();
    if(( currenttime - lastspindownstep ) > SpinDownStep )
    {
      lastspindownstep = millis();
      CalcMotorSpeed -= SpinDownSteps[SpinDown_i];
      if( CalcMotorSpeed <= thirtypcMotorSpeed )
      {
        StopMotors();
        SpinDown = false;
        SpinDown_i = 0;
        
      }
      else
      {
        MainMotor1.writeMicroseconds( CalcMotorSpeed );
        MainMotor2.writeMicroseconds( CalcMotorSpeed );     
        SpinDown_i++;    
      }
  
    }
  }
}










void setup() {

  unsigned long BootStart = millis();
  Serial.begin( 115200 );
//  while (!Serial);
//  delay(100);

pinMode(SWPin, INPUT_PULLUP);

// Input Pins
  pinMode( PIN_REV, INPUT_PULLUP );
  pinMode( PIN_FIRE, INPUT_PULLUP );
  pinMode( PIN_SELE, INPUT_PULLUP );
  pinMode( PIN_PUSH, INPUT_PULLUP );


  // Output Pins
  pinMode( PIN_EN, OUTPUT );
  pinMode( PIN_PH, OUTPUT );
  digitalWrite( PIN_PH, HIGH);    // Set PH high - foward. Set to low for rev
  pinMode( PIN_ESC1, OUTPUT );
  pinMode( PIN_ESC2, OUTPUT );





  // Brushless  Setup
  MainMotor1.attach(PIN_ESC1);
  MainMotor2.attach(PIN_ESC2);
  // Arm ESCs
  MainMotor1.writeMicroseconds(MinMotorSpeed);
  MainMotor2.writeMicroseconds(MinMotorSpeed);
  delay(3000); // Wait for ESC to initialise

  SpinDown = false;
  
 PushButtonState = BTN_LOW;

PWM_Instance = new RP2040_PWM(PIN_EN, freq, dutyCycle_off);
if (PWM_Instance)
{
  PWM_Instance->setPWM();
}


  // Neopixel Setup
    pixels.begin();

  pinMode(PIN_RGB_PWR,OUTPUT);

  digitalWrite(PIN_RGB_PWR, HIGH);
    pixels.clear();

  pixels.setPixelColor(0, pixels.Color(0, 0, 255));


  Serial.print(F("\nStarting Flycore Brushless on ")); Serial.println(BOARD_NAME);
  Serial.println(RPI_PICO_TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Interval in microsecs
  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1))
  {
    Serial.print(F("Starting ITimer1 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

  if (ITimer2.attachInterruptInterval(TIMER2_INTERVAL_MS * 1000, TimerHandler2))
  {
    Serial.print(F("Starting ITimer2 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer2. Select another freq. or timer"));


}

void loop() {


  delay(1);        // this delay is here to slow down the code for debugging. remove when finalised.

  ProcessSelector();
  ProcessFire();
  ProcessMotors();
//  buttonstatus();
}

void buttonstatus()
{
  Serial.println("---");
  Serial.print(F("Pusher: ")); Serial.println(digitalRead( PIN_PUSH ));
  Serial.print(F("Fire: ")); Serial.println(digitalRead( PIN_FIRE ));
  Serial.print(F("Rev: ")); Serial.println(digitalRead( PIN_REV ));
  Serial.print(F("Select: ")); Serial.println(digitalRead( PIN_SELE ));
}
