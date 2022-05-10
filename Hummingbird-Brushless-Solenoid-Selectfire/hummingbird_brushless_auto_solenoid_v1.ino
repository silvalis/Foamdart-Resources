

/* 
 *  Hummingbird brushless select solenoid
 *  for Arduino nano atmega 328P
 *  FTW Hyperdrive
 *  Turnigy Multistar 33A blheli_32
 *  Brother Hobby R6 2205 2700kV
 *  Trigger, Select
 *  
 *  This is an adapted version of the Gryphon brushed select solenoid by DT
 *  This is an adapted version of the Airzonesama Narfduino Brushed Auto Solenoid program
 *  
 *  author dt
 *  
 *  v0.1 Draft 5/5/2022
 *  v1.0 Initial Release 21/04/2022
 *  
 */

/*
 * Pin Definitions
 * 
 * INPUTS
 * -------
 * PD2/D2 Trigger
 * PD3/D3 Select Toggle
 * 
 * 
 * 
 * OUTPUTS
 * -------
 * PD6/D6 Mosfet Run / Solenoid
 * PB1/D9 ESC 1
 * PB2/D10 ESC 2
 * 
 */

#include <Servo.h>
#define MotorKV 2700 


#define PIN_TRIGGER 3
#define PIN_SEL1 2

#define PIN_RUN 6
#define PIN_ESC1 9
#define PIN_ESC2 10


Servo MainMotor1;
Servo MainMotor2;

// Motor Settings
#define MOTOR_SPINUP_LAG 150 // How long we give the motors before we know that have spun up.
#define MOTOR_SPINDOWN_3S 400
#define MOTOR_SPINUP_3S 150
int MaxMotorSpeed = 2000;
int DecelerateTime = MOTOR_SPINDOWN_3S; //0;
int AccelerateTime = MOTOR_SPINUP_3S; //0;
int MotorRampUpPerMS = 0;
int MotorRampDownPerMS = 0;
int MinMotorSpeed = 1000;
int CurrentMotorSpeed = MinMotorSpeed;
int TargetMotorSpeed = 1850;
bool MotorsEnabled = false;
byte SetMaxSpeed = 100; // in percent.
unsigned long TimeLastMotorSpeedChanged = 0;
#define COMMAND_REV_NONE 0
#define COMMAND_REV_HALF 1
#define COMMAND_REV_FULL 2
int CommandRev = COMMAND_REV_NONE;
int PrevCommandRev = COMMAND_REV_NONE;

unsigned long StopMillis;


// Pin macros
/*
 * Read pin state
 * 
 * Valid for:
 * PD2/D2 - Trigger 1  FIRE_T
 * PD3/D3 Select Toggle
 * 
 */
#define GET_FIRE_T ((PIND & 0b00001000) ? HIGH : LOW )
#define GET_SEL1 ((PIND & 0b00000100) ? HIGH : LOW )

/*
 * Set Pin State
 * 
 * Valid for:
 * PD6/D6 Mosfet Run / Solenoid
 * PB1/D9 ESC 1
 * PB2/D10 ESC 2
 */
//#define SET_P_HIGH_FULL_ON (PORTD |= 0b00000010)
//#define SET_P_HIGH_FULL_OFF (PORTC &= 0b11111101)
#define SET_P_LOW_FULL_ON (PORTD |= 0b01000000)
#define SET_P_LOW_FULL_OFF (PORTD &= 0b10111111)
#define SET_ESC_FULL_ON (PORTB |= 0b00000110)
#define SET_ESC_FULL_OFF (PORTB &= 0b11111001)


// System Modes
#define SYSTEM_MODE_NORMAL 0
#define SYSTEM_MODE_LOWBATT 1
#define SYSTEM_MODE_FLYWHEELFAIL 2
byte SystemMode = SYSTEM_MODE_NORMAL;


// Firing Controls
#define MIN_DWELL_TIME 1 //20
#define DPS 50
#define BURST_SIZE 2

// Inputs
#define DebounceWindow 5 // Debounce Window = 5ms
//#define RotaryDebounceWindow 100 // Report window for rotary encoder changes
#define RepollInterval 250 // Just check the buttons ever xxx just in case we missed an interrupt.
// For ISR use
volatile bool Trigger1Changed = false;
volatile bool Sel1Changed = false;
#define BTN_LOW 0
#define BTN_HIGH 1
#define BTN_ROSE 2
#define BTN_FELL 3
byte Trigger1ButtonState = BTN_HIGH;
byte Sel1ButtonState = BTN_HIGH;
volatile byte LastPINB = 0; // Preload with PINB Prior to events happening
volatile byte LastPIND = 0; // Preload with PIND Prior to events happening
volatile bool PusherTickTock = false; // To capture the first edge of each in / out cycle

int SF = 0;   // select fire mode. this just tick up and gets mod3'd


void setup() {
  // put your setup code here, to run once:

  // Just for any initial timing..
  unsigned long BootStart = millis();
  Serial.begin( 9600 );
  
  // PCINT Inputs
  // PB - PCINT Vector 0
  // PD - PCINT Vector 2
  pinMode(PIN_TRIGGER, INPUT_PULLUP); 
  PCMSK2 |= (1 << PCINT19);       // PD3/D3/PCINT19

    // SelectFire 1
  pinMode( PIN_SEL1, INPUT_PULLUP );
  PCMSK2 |= (1 << PCINT18);  //PD3/D3 Select Toggle

  // Capture the PINx state and turn on the PCINT interrupts
  LastPIND = PIND; 
  PCICR |= (1 << PCIE2); // Activates control register for PCINT vector 2  


// Brushless Motors
MainMotor1.attach(PIN_ESC1);
MainMotor2.attach(PIN_ESC2);
// Arm ESCs
MainMotor1.writeMicroseconds(MinMotorSpeed);
MainMotor2.writeMicroseconds(MinMotorSpeed);
delay(3000); // Wait for ESC to initialise


CalculateRampRates();


  // Setup Pusher Outputs
  pinMode( PIN_RUN, OUTPUT );
  SET_P_LOW_FULL_OFF;
//  pinMode( PIN_STOP, OUTPUT );
//  SET_P_HIGH_FULL_OFF;   // Leave this off forever.
  pinMode( PIN_ESC1, OUTPUT );
//  SET_ESC_FULL_OFF;   // Do both at the same time?
  pinMode( PIN_ESC2, OUTPUT );
  SET_ESC_FULL_OFF;
//  SET_MOTOR_FULL_OFF;


  // Wait for the sync - 1 seconds, but run the button debouncer to capture a trigger down state for EEPROM reset
  while( millis() - BootStart < 2000 )
  {
    ProcessDebouncing();
    delay( 10 );
  }

  // Wait for the user to pull their finger off the trigger
  while( (Trigger1ButtonState == BTN_LOW) || (Trigger1ButtonState == BTN_FELL) )
  {
    ProcessDebouncing();
    delay(10);
  }
  delay(10);
  
}

void loop() {
  //Serial.println(digitalRead(PIN_TRIGGER));
//  Serial.println(digitalRead(PIN_SEL1));

  // put your main code here, to run repeatedly:
  ProcessDebouncing(); // Process the pin input, and handle any debouncing
//  ProcessBatteryMonitor(); // Check battery voltage occasionally
//  ProcessADC(); // Handle the ADC reading
  ProcessSystemMode(); // Handle the system mode

  



  
  ProcessFiring(); // Handle any firing here
  ProcessSelect(); // Handle Select Button
}





// ISR Sections



// PCINT - PB

// PCINT - PD
ISR( PCINT2_vect ) // PD
{
   
  // Trigger 1 on PD3
  if( ((PIND & 0b00001000) == 0b00001000) && ((LastPIND & 0b00001000) != 0b00001000) )
  {
    Trigger1Changed = true;
  }

//  LastPIND = PIND;


  // Sel 1 on PD2
  if( ((PIND & 0b00000100) == 0b00000100) && ((LastPIND & 0b00000100) != 0b00000100) )
  {
    Sel1Changed = true;
  }

  LastPIND = PIND;
  
}





// Process the debouncing of the directly connected MCU inputs
void ProcessDebouncing()
{
  // Single call to millis() for better performance
  unsigned long CurrentMillis = millis();
  
  /*
   * Trigger 1
   */
  static bool RunTrigger1Timer = false;
  static unsigned long LastTrigger1Press = 0;
  static byte Trigger1DebounceState = 0;
  // Set up a repoll interval, just in case the interrupt is missed.
  if( CurrentMillis - LastTrigger1Press > RepollInterval ) Trigger1Changed = true; 
  // Move from edge to steady state
  if( Trigger1ButtonState == BTN_ROSE ) Trigger1ButtonState = BTN_HIGH;
  if( Trigger1ButtonState == BTN_FELL ) Trigger1ButtonState = BTN_LOW;  
 // Serial.println(Trigger1Changed);
  if( Trigger1Changed )
  {
   Trigger1Changed = false; 
   if( !RunTrigger1Timer )
   {
    LastTrigger1Press = CurrentMillis;

    Trigger1DebounceState = 0b01010101; // We need to give the button time to settle. This will track. 
    RunTrigger1Timer = true;    
   }   
  }
  if( RunTrigger1Timer && ( CurrentMillis - LastTrigger1Press > DebounceWindow  ) )
  {
    Trigger1DebounceState = (Trigger1DebounceState << 1) | ((PIND >> 3) & 0b00000001); // Shift the register pin to the left, Shift the pin result to the right most position, and tack it onto the debounce state. Ensure that only the last position can be a 1.

    if( (Trigger1DebounceState == 0) || (Trigger1DebounceState == 255) ) // All 0's or all 1's. This means we have settled.
    {
      RunTrigger1Timer = false;
    
      if( GET_FIRE_T )
      { 
        if( Trigger1ButtonState != BTN_HIGH ) Trigger1ButtonState = BTN_ROSE;
    //    Serial.println("Fire");
      }
      else
      { 
        if( Trigger1ButtonState != BTN_LOW ) Trigger1ButtonState = BTN_FELL;
   //     Serial.println("NotFire");
      }    
    }
  }
/*
   * Sel 1
   */
  static bool RunSel1Timer = false;
  static unsigned long LastSel1Press = 0;
  static byte Sel1DebounceState = 0;
  // Set up a repoll interval, just in case the interrupt is missed.
  if( CurrentMillis - LastSel1Press > RepollInterval ) Sel1Changed = true; 
  // Move from edge to steady state
  if( Sel1ButtonState == BTN_ROSE ) Sel1ButtonState = BTN_HIGH;
  if( Sel1ButtonState == BTN_FELL ) Sel1ButtonState = BTN_LOW;  
  if( Sel1Changed )
  {
   Sel1Changed = false; 
   if( !RunSel1Timer )
   {
    LastSel1Press = CurrentMillis;

    Sel1DebounceState = 0b01010101; // We need to give the button time to settle. This will track. 
    RunSel1Timer = true;    
   }   
  }
  if( RunSel1Timer && ( CurrentMillis - LastSel1Press > DebounceWindow  ) )
  {
    Sel1DebounceState = (Sel1DebounceState << 1) | ((PIND >> 2) & 0b00000001); // Shift the register pin to the left, Shift the pin result to the right most position, and tack it onto the debounce state. Ensure that only the last position can be a 1.

    if( (Sel1DebounceState == 0) || (Sel1DebounceState == 255) ) // All 0's or all 1's. This means we have settled.
    {
      RunSel1Timer = false;
    
      if( GET_SEL1 )
      { 
        if( Sel1ButtonState != BTN_HIGH ) Sel1ButtonState = BTN_ROSE;
      }
      else
      { 
        if( Sel1ButtonState != BTN_LOW ) Sel1ButtonState = BTN_FELL;
      }    
    }
  }

  
}


// Basic system mode implementation. If a flywheel jam is detected, then reset after a time-out.
void ProcessSystemMode()
{
  static byte LastSystemMode = 99;
  static unsigned long LastFlywheelJam = 0;
  
//  if( BatteryFlat )
//  {
//    SystemMode = SYSTEM_MODE_LOWBATT;
//  }
//  else 
//  {
    SystemMode = SYSTEM_MODE_NORMAL;
//  }
//
  LastSystemMode = SystemMode;
}

void ProcessSelect()
{
  if( Sel1ButtonState == BTN_ROSE )
  {
        SF = (SF + 1) % 3;
        Serial.println(SF);
  }
}

/* 
 * Main firing control logic. Use a blocking technique. 
 */ 
void ProcessFiring()
{
  bool StartFiringCycle = false;
  static unsigned long RevStart = 0;





  // Check to see if the trigger fell
  if( Trigger1ButtonState == BTN_FELL ){
    StartFiringCycle = true;   
//Serial.println(Trigger1ButtonState);
  }




  // Fire if we must
  if( StartFiringCycle )
  {
    // User pressed trigger

    //Evaluate Select Fire 'sf'

//    byte SFA = GET_SEL1;
//    byte SFB = GET_SEL2;
    #define AUTO 2
    #define BURST 1
    #define SINGLE 0
    byte CurrentFireMode = SINGLE;



    if( SF == 2 )
    {
      CurrentFireMode = AUTO;
    }
    else if( SF == 1 )
    {
      CurrentFireMode = BURST;

    }
    else
    {
      CurrentFireMode = SINGLE;
    }

    // Start motors
    RevStart = millis();
    StartMotors();
    if( millis() - StopMillis < AccelerateTime )
    {
      AccelerateTime = MOTOR_SPINUP_3S - (millis() - RevStart);
    }
    else
    {
      AccelerateTime = MOTOR_SPINUP_3S;
    }
    while( millis() - RevStart < AccelerateTime )
    {
      delay(1);
    }
    // Implement auto cycling using a post-test loop.. That way the cycle will always fire once for single shot.
    int CurrentShot = 0;
    do
    {
      FireSolenoid();
      ProcessDebouncing();
      Serial.println( CurrentShot);
      CurrentShot++;
    } while( (CurrentFireMode == AUTO && GET_FIRE_T == LOW) || (CurrentFireMode == BURST && CurrentShot < BURST_SIZE && GET_FIRE_T == LOW) );
    StopMotors();
  }
}

// The solenoid firing sequence.
void FireSolenoid()
{

  unsigned long StartTimer = millis();


//Serial.println(">>>Solenoid On");
  // Turn On Solenoid
  SET_P_LOW_FULL_ON;


  // 50ms for pusher to extend. 50+50=100ms = 10dps
  StartTimer = millis();

  while( millis() - StartTimer < 50 )
  {
    delay(1);
  }  
 
  // Turn Off Solenoid
//Serial.println("Solenoid Off");
  SET_P_LOW_FULL_OFF;

  

  StartTimer = millis();
// 30ms delay for pusher to retract (remmeber its got a spring)
  while( millis() - StartTimer < 50 )
  {
    delay(1);

  }  

}

void StartMotors()
{
 // SET_ESC_FULL_ON;


    MainMotor1.writeMicroseconds( TargetMotorSpeed );
    MainMotor2.writeMicroseconds( TargetMotorSpeed );
 
}

void StopMotors()
{
//  SET_ESC_FULL_OFF;  



    MainMotor1.writeMicroseconds( MinMotorSpeed );
    MainMotor2.writeMicroseconds( MinMotorSpeed );
    StopMillis = millis();

}





void CalculateRampRates()
{
  long SpeedRange = (long)(MaxMotorSpeed - MinMotorSpeed) * 1000; // Multiply by 1000 to give some resolution due to integer calculations
  if( AccelerateTime == 0 )
  {
    MotorRampUpPerMS = SpeedRange;  // For instant acceleration
  }
  else
  {
    MotorRampUpPerMS = SpeedRange / AccelerateTime;  // Use when Accelerating
  }

  if( DecelerateTime == 0 )
  {
    MotorRampDownPerMS = SpeedRange;  // For instant acceleration
  }
  else
  {
    MotorRampDownPerMS = SpeedRange / DecelerateTime;  // Use when Decelerating
  }  


  Serial.print( "Ramp Up per MS = " );
  Serial.println( MotorRampUpPerMS );

  Serial.print( "Ramp Down per MS = " );
  Serial.println( MotorRampDownPerMS );

  Serial.print( "AccelerateTime = " );
  Serial.println( AccelerateTime );

  Serial.print( "DecelerateTime = " );
  Serial.println( DecelerateTime );

  Serial.print( "SpeedRange = " );
  Serial.println( SpeedRange );
}
