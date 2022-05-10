

/* 
 *  Gryphon brushed select solenoid
 *  for Narfduino
 *  FTW Hyperdrive
 *  NeoCat Daybreak
 *  Rev, Trigger, centre off SPDT
 *  
 *  This is an adapted version of the Airzonesama Narfduino Brushed Auto Solenoid program
 *  
 *  author dt
 *  
 *  v0.1 Draft 21/04/2022
 *  v1.0 Initial Release 21/04/2022
 *  
 */

/*
 * Pin Definitions
 * 
 * INPUTS
 * -------
 * PD6/6 Trigger
 * PB1/D9 Rev
 * PB3/D11 Select 1
 * PB4/D12 Select 2
 * 
 * OUTPUTS
 * -------
 * PD5/D5 Mosfet Run 
 * PC1/D15 Brake
 * PD4/D4 Flywheel Motor
 * 
 */



#define PIN_TRIGGER 6
#define PIN_REV 9
#define PIN_SEL1 11
#define PIN_SEL2 12

#define PIN_RUN 5
#define PIN_STOP 15
#define PIN_MOTOR 4



// Pin macros
/*
 * Read pin state
 * 
 * Valid for:
 * PD6/D6 - Trigger 1  FIRE_T
 * PB1/D9 - Trigger 2  REV
 * PB3/D11 - Select 1
 * PB4/D12 - Select 2
 * 
 */
#define GET_FIRE_T ((PIND & 0b01000000) ? HIGH : LOW )
#define GET_REV ((PINB & 0b00000010) ? HIGH : LOW )
#define GET_SEL1 ((PINB & 0b00001000) ? HIGH : LOW )
#define GET_SEL2 ((PINB & 0b00010000) ? HIGH : LOW )

/*
 * Set Pin State
 * 
 * Valid for:
 * PC1 P-BRAKE
 * PD5 P-RUN
 * PD4 Motor
 */
#define SET_P_HIGH_FULL_ON (PORTC |= 0b00000010)
#define SET_P_HIGH_FULL_OFF (PORTC &= 0b11111101)
#define SET_P_LOW_FULL_ON (PORTD |= 0b00100000)
#define SET_P_LOW_FULL_OFF (PORTD &= 0b11011111)
#define SET_MOTOR_FULL_ON (PORTD |= 0b00010000)
#define SET_MOTOR_FULL_OFF (PORTD &= 0b11101111)


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
volatile bool RevChanged = false;
#define BTN_LOW 0
#define BTN_HIGH 1
#define BTN_ROSE 2
#define BTN_FELL 3
byte Trigger1ButtonState = BTN_HIGH;
byte RevButtonState = BTN_HIGH;
volatile byte LastPINB = 0; // Preload with PINB Prior to events happening
volatile byte LastPIND = 0; // Preload with PIND Prior to events happening
volatile bool PusherTickTock = false; // To capture the first edge of each in / out cycle


void setup() {
  // put your setup code here, to run once:

  // Just for any initial timing..
  unsigned long BootStart = millis();
  Serial.begin( 9600 );
  
  // PCINT Inputs
  // PB - PCINT Vector 0
  // PD - PCINT Vector 2
  pinMode(PIN_TRIGGER, INPUT); 
  PCMSK2 |= (1 << PCINT22);

  // Capture the PINx state and turn on the PCINT interrupts
  LastPIND = PIND; 
  PCICR |= (1 << PCIE2); // Activates control register for PCINT vector 2  



  // SelectFire 1 and 2 Switch
  pinMode( PIN_SEL1, INPUT_PULLUP );
  pinMode( PIN_SEL2, INPUT_PULLUP );

  // Rev switch
  pinMode( PIN_REV, INPUT );


  // Setup Pusher Outputs
  pinMode( PIN_RUN, OUTPUT );
  SET_P_LOW_FULL_OFF;
  pinMode( PIN_STOP, OUTPUT );
  SET_P_HIGH_FULL_OFF;   // Leave this off forever.
  pinMode( PIN_MOTOR, OUTPUT );
  SET_MOTOR_FULL_OFF;


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

  // put your main code here, to run repeatedly:
  ProcessDebouncing(); // Process the pin input, and handle any debouncing
//  ProcessBatteryMonitor(); // Check battery voltage occasionally
//  ProcessADC(); // Handle the ADC reading
  ProcessSystemMode(); // Handle the system mode
  ProcessFiring(); // Handle any firing here
}





// ISR Sections



// PCINT - PB

// PCINT - PD
ISR( PCINT2_vect ) // PD
{
   
  // Trigger 1 on PD6
  if( ((PIND & 0b01000000) == 0b01000000) && ((LastPIND & 0b01000000) != 0b01000000) )
  {
    Trigger1Changed = true;
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
    Trigger1DebounceState = (Trigger1DebounceState << 1) | ((PIND >> 6) & 0b00000001); // Shift the register pin to the left, Shift the pin result to the right most position, and tack it onto the debounce state. Ensure that only the last position can be a 1.

    if( (Trigger1DebounceState == 0) || (Trigger1DebounceState == 255) ) // All 0's or all 1's. This means we have settled.
    {
      RunTrigger1Timer = false;
    
      if( GET_FIRE_T )
      { 
        if( Trigger1ButtonState != BTN_HIGH ) Trigger1ButtonState = BTN_ROSE;
      }
      else
      { 
        if( Trigger1ButtonState != BTN_LOW ) Trigger1ButtonState = BTN_FELL;
      }    
    }
  }


    /*
   * Rev
   */
  static bool RunRevTimer = false;
  static unsigned long LastRevPress = 0;
  static byte RevDebounceState = 0;
  // Set up a repoll interval, just in case the interrupt is missed.
  if( CurrentMillis - LastRevPress > RepollInterval ) RevChanged = true; 
  // Move from edge to steady state
  if( RevButtonState == BTN_ROSE ) RevButtonState = BTN_HIGH;
  if( RevButtonState == BTN_FELL ) RevButtonState = BTN_LOW;  
  if( RevChanged )
  {
  RevChanged = false; 
   if( !RunRevTimer )
   {
    LastRevPress = CurrentMillis;

    RevDebounceState = 0b01010101; // We need to give the button time to settle. This will track. 
    RunRevTimer = true;    
   }   
  }
  if( RunRevTimer && ( CurrentMillis - LastRevPress > DebounceWindow  ) )
  {
    RevDebounceState = (RevDebounceState << 1) | ((PINB >> 1) & 0b00000001); // Shift the register pin to the left, Shift the pin result to the right most position, and tack it onto the debounce state. Ensure that only the last position can be a 1.

    if( (RevDebounceState == 0) || (RevDebounceState == 255) ) // All 0's or all 1's. This means we have settled.
    {
      RunRevTimer = false;
    
      if( GET_REV )
      { 
        if( RevButtonState != BTN_HIGH ) RevButtonState = BTN_ROSE;
      }
      else
      { 
        if( RevButtonState != BTN_LOW ) RevButtonState = BTN_FELL;
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

/* 
 * Main firing control logic. Use a blocking technique. 
 */ 
void ProcessFiring()
{
  bool StartFiringCycle = false;
  static unsigned long RevStart = 0;

  // Check to see if the rev fell
  if( RevButtonState == BTN_LOW )
  {
    StartMotors();

  }
  else 
  {
    StopMotors();

  }




  // Check to see if the trigger fell
  if( Trigger1ButtonState == BTN_FELL )
    StartFiringCycle = true;   






  // Fire if we must
  if( StartFiringCycle )
  {
    // User pressed trigger

    //Evaluate Select Fire
    byte SFA = GET_SEL1;
    byte SFB = GET_SEL2;
    #define AUTO 0
    #define BURST 1
    #define SINGLE 2
    byte CurrentFireMode = SINGLE;



    if( SFA == LOW && SFB == HIGH )
    {
      CurrentFireMode = SINGLE;
      //CurrentFireMode = AUTO;
    }
    else if( SFA == HIGH && SFB == LOW )
    {
//      CurrentFireMode = BURST;
      CurrentFireMode = AUTO;
    }
    else
    {
      CurrentFireMode = BURST;
//      CurrentFireMode = AUTO;
    }

    // Start motors
//    RevStart = millis();
//    StartMotors();
//    while( millis() - RevStart < 200 )
//    {
//      delay(1);
//    }
    // Implement auto cycling using a post-test loop.. That way the cycle will always fire once for single shot.
    int CurrentShot = 0;
    do
    {
      FireSolenoid();
      ProcessDebouncing();
      CurrentShot++;
    } while( (CurrentFireMode == AUTO && GET_FIRE_T == LOW) || (CurrentFireMode == BURST && CurrentShot < BURST_SIZE && GET_FIRE_T == LOW) );
//    StopMotors();
  }
}

// The solenoid firing sequence.
void FireSolenoid()
{

  unsigned long StartTimer = millis();


Serial.println(">>>Solenoid On");
  // Turn On Solenoid
  SET_P_LOW_FULL_ON;


  // 70ms for pusher to extend. 70+30=100ms = 10dps
  StartTimer = millis();

  while( millis() - StartTimer < 50 )
  {
    delay(1);
  }  
 
  // Turn Off Solenoid
Serial.println("Solenoid Off");
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
  SET_MOTOR_FULL_ON;
}

void StopMotors()
{
  SET_MOTOR_FULL_OFF;  
}
