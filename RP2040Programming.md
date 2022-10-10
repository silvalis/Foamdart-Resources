# Raspberry Pico RP2040 Wiring and Programming Guide
 
 
 ---------------
 If you like my work, please buy me a coffee!
 https://www.buymeacoffee.com/silvalisdesigns
 ---------------
 
 
# To Do

- Pictures
- State machine
- Debouncer
- Stuff


 
# Steps to a cool MCU controlled blaster

1) Select your hardware
2) Draw a diagram and label your pins
3) Breadboard it first
4) Get it working
5) Mock wire it, get your wire lengths right
6) Loom/prewire
7) Pew

# The RP2040

It's an ARM Cortex M0+ mcu. 32bit. Hurray. It's mini.

Comes in several sizes
Pico

Seeed Xiao RP2040, Adafruit QT PY RP2040

DFRobot Beetle RP2040 Mini

Advantages:
Mini
Hardware PIO (Runs independent from main processor)

Disadvantages:
3V3 or 5V input required (Separate regulator)

# Which one do I pick?

For this form factor, pick based on required input/outputs.


Example 1: Brushed Flywheel and Solenoid Pusher

INPUT: Flywheel Rev
INPUT: Pusher
OUTPUT: Flywheel MOSFET
OUTPUT: Pusher MOSFET

4 i/o required

The DFRobot Beetle (8 GPIO), Seeed Xiao RP2040 (10 GPIO), Adafruit QT PY RP2040 (10 GPIO)
 
Example 2: Brushless and Select Fire Solenoid Pusher

INPUT: Flywheel Rev
INPUT: Pusher
INPUT: Select Fire Switch
OUTPUT: ESC1
OUTPUT: ESC2
OUTPUT: Pusher MOSFET

6 i/o required

The DFRobot Beetle (8 GPIO), Seeed Xiao RP2040 (10 GPIO), Adafruit QT PY RP2040 (10 GPIO)



Example 2: Brushless and Auto/Burst/Semi Select Fire Motorised Pusher

INPUT: Flywheel Rev
INPUT: Pusher
INPUT: Pusher Positional Switch
INPUT: Select Fire Switch Auto
INPUT: Select Fire Switch Burst
INPUT: Select Fire Switch Semi
OUTPUT: ESC1
OUTPUT: ESC2
OUTPUT: DC Motor Driver EN
OUTPUT: DC Motor Driver PH

10 i/o required

The Seeed Xiao RP2040 (10 GPIO), Adafruit QT PY RP2040 (10 GPIO)
 
 ---------------
 
# Hardware Selection

RP2040

<10 GPIO

DFRobot Beetle RP2040 Mini 8GPIO 
https://www.dfrobot.com/product-2615.html

Note: The Beetle RP2040 has 8 GPIO while the Seeed Xiao and Adafruit QT PY are better specced with 10. The Beetle is larger than the other two. If you are new to soldering PCBs, the Beetle has large easy to solder pads which is useful.

Seeed Xiao RP2040 $6

Adafruit 4900 QT Py RP2040, Stemma QT - $10 
https://www.adafruit.com/product/4900


21 GPIO

Adafruit 4884 Feather, Stemma QT - $12
https://www.adafruit.com/product/4884


30 GPIO

Adafruit 4864 RPI Pico - $4
https://www.adafruit.com/product/4864


VReg

Pololu 2843, 5V, 500mA  
https://www.pololu.com/product/2843

Pololu 2842, 3V3, 500mA  
https://www.pololu.com/product/2842


Switches

Any switches - no current requirement for input switches.





ESC

BLHeli_32 

BLHeli_S





MOSFET Driver

Freetronics N-MOSFET Driver AUD$4
https://www.freetronics.com.au/collections/modules/products/n-mosfet-driver-output-module

Any N-MOSFET that has a low gate switch on voltage (Vgs)





DC Motor Driver (eg, N20)

Pololu 4036, DRV8876, $7
https://www.pololu.com/product/4036








 ---------------
 
# Wiring Considerations


*** CHECK YOUR CURRENT REQUIREMENTS ***
This is a rough guide:

Signalling: You can use 22AWG to 28AWG wiring on these boards. Be careful with 28AWG, you may snap the wires with excessive movement.

RP2040 Power: 22AWG to 24AWG

Mosfet Drivers: 18AWG to 20AWG should be fine.

ESCs will typically need 14AWG to 18AWG, depending on motor requirements.


# Switch wiring

The simplest method is to wire one terminal of the switch to the GPIO pin and the other to Ground. 

This means when the switch is 

-Closed/ground = LOW (0)
-Open/floating = HIGH (1)


NC or NO? It really doesn't matter.

NC Pros:
Possibility of detecting switch failure on startup

NO Pros:
Less current consumption than NC since the switch will spend most of its time not pressed


# MOSFET wiring

Attach the MOSFET Gate to the GPIO pin. 


# ESC connection 

If you are using brushless ESCs, consider keeping the dupont connector in the wiring loom so that you can easily reprogram with BLHeliSuite if required.

 ---------------
  
# Programming the RP2040

This tutorial is focused around Arduino IDE, but you can use Visual Studio Code, or program it in micropython or circuitpython.

Use this arduino board library. It works better than the others.
https://github.com/earlephilhower/arduino-pico



 
# General

I suggest when assigning pin mapping for RP2040, use the GPIO port number rather than shortcut designations (eg, for GPIO29, use 29 instead of D0). The library may not map correctly and while it will compile, your code might not work.



General code program flow - state machine based

Poll inputs

Change internal state model based on input

Write to outputs




# Input Switches

Use the digitalRead(pin) function to read the state of a pin, eg your switch
If you directly wire in your switches, use the internal PULLUP (or PULLDOWN depending on your wiring).
The following example toggles the built in LED on/off with the switch

```
int rev_pin = 28;
setup() {
	pinMode( rev_pin, INPUT_PULLUP );
	pinMode( LED_BUILTIN, OUTPUT );		// Assuming your board has a mapped built in LED
}
loop() {
	if ( digitalRead( rev_pin ) == HIGH );
	{
		digitalWrite(LED_BUILTIN, HIGH);		// ON
	}
	else
	{		
		digitalWrite(LED_BUILTIN, LOW);
	}
}
```

# Brushless ESCs

Brushless ESCs utilise servo pwm  -we set them up to have 0% = 1040us, 100% = 2040us or similar. My setting recommendations

% ---- BLHeli -- Code
0% --- 1040us -- 1000us
100% - 1960us -- 2000us

I've had some issues where the pulse width generated will not arm the ESC properly when trying to hit a specific number. It's easier to just overshoot the width in code.

Basic usage:
```
#include "Servo.h"
int esc_pin = 29;	// GPIO port number
int rev_pin = 28;
Servo myServo;

void setup() {
	pinMode( esc_pin, OUTPUT );
	pinMode( rev_pin, INPUT_PULLUP );
	myServo.attach( esc_pin );
	myServo.writeMilliseconds( 1000 ); 	// OFF
}
void loop() {
	if ( digitalRead( rev_pin ) == HIGH );
	{
		myServo.writeMilliseconds( 2000 ); 	// 100%
	}
	else
	{		
		myServo.writeMilliseconds( 1000 ); 	// OFF
	}
}

```

Troubleshooting:

ESCS do not work:
BLHeli_32 units play a tone on startup.
First 3 tones (rising pitch) = startup
4th tone (low) = start arming
5th tone (high) = arm completed, throttle on 0%

Identify where the startup tone stops. Eg, if the 5th tone is not played, it's most likely that your code does not set throttle to 0%.


# Solenoid Pusher

If you use a mosfet driver for the solenoid pusher, simply program it similar to

Basic usage:
```
int solenoid_pin = 29;	// GPIO port number
int pusher_pin = 28;

void setup() {
	pinMode( solenoid_pin, OUTPUT );
	pinMode( pusher_pin, INPUT_PULLUP );
	digitalWrite( solenoid_pin, LOW ); 	// OFF
	
}
void loop() {
	if ( digitalRead( pusher_pin ) == HIGH );
	{
		digitalWrite( solenoid_pin, HIGH );	// ON
	}
	else
	{		
		digitalWrite( solenoid_pin, LOW ); 	// OFF
	}
}

```


# RP2040 Hardware PWM

If you are using a mosfet/motor driver pusher, you can (generally) use pwm to control the speed of the motor. I used this on my DC Motor Driver board.

Each pin on the RP2040 has proper pwm capability. There are some limitations on this - pins share PWM, but in general it doesnt matter.


Use this library
Hardware PWM: Khoi Hoang https://github.com/khoih-prog/RP2040_PWM

```
#include "RP2040_PWM.h"
#define pwm_pin 29   // GPIO port number
#define pusher_pin 28  
RP2040_PWM* PWM_Instance;

float freq = 20000.0f;	// 20kHz freq
float dutyCycle_off = 0.0f;		// off
float dutyCycle_full = 100.0f;	// on

void setup()
{
	pinMode( pwm_pin, OUTPUT );
	pinMode( pusher_pin, INPUT_PULLUP );
	PWM_Instance = new RP2040_PWM( pwm_pin, freq, dutyCycle_off );
	if( PWM_Instance )
	{
		PWM_Instance->setPWM();
	}
}
void loop()
{
	if( digitalRead( pusher_pin ) == HIGH );		// ON
	{
		PWM_Instance = new RP2040_PWM( pwm_pin, freq, dutyCycle_full ;
		if( PWM_Instance )
		{
			PWM_Instance->setPWM();
		}
	}
	else		// OFF
	{		
		PWM_Instance = new RP2040_PWM( pwm_pin, freq, dutyCycle_off );
		if( PWM_Instance )
		{
			PWM_Instance->setPWM();
		}
	}
}  
  
```



# Interrupt Service Routine (ISR)

ISRs are for mission critical code that needs to be run, even if the processor is busy

ISR Servo

You don't really need to ISR the servo, but I guess it's nice
This is a replacement for the other ESC Servo above





```
#include "RP2040_ISR_Servo.h"
#define esc1_pin        29   	// Servo
#define esc2_pin        28    	// Servo
#define rev_pin 		27		// Input pin  
int esc_full = 2000;
int esc_off = 1000;

//ISR servos for rp2040
typedef struct
{
	int     servoIndex;
	uint8_t servoPin;
} 	ISR_servo_t;

#define NUM_SERVOS            2

ISR_servo_t ISR_servo[NUM_SERVOS] =
{
	{ -1, esc1_pin }, { -1, esc2_pin }
};

void setup()
{

	for( int index = 0; index < NUM_SERVOS; index++ )
	{
		pinMode(ISR_servo[index].servoPin, OUTPUT);
		digitalWrite(ISR_servo[index].servoPin, LOW);
	}
  	pinMode( rev_pin, INPUT_PULLUP );
}
void loop()
{
	if( digitalRead( rev_pin ) == HIGH );		// Turn ON 100%
	{
		for( int index = 0; index < NUM_SERVOS; index++ )
	    {
			RP2040_ISR_Servos.writeMicroseconds( ISR_servo[index].servoIndex, esc_full );
		}
	}
	else		// Turn OFF 0%
	{		
		for( int index = 0; index < NUM_SERVOS; index++ )
	    {
			RP2040_ISR_Servos.writeMicroseconds( ISR_servo[index].servoIndex, esc_off );
	    }
	}
}

```






# Programmable IO (PIO) 

todo

We like using PIO because it's essentially a separate bit of code, or uninterruptable program. 

# ISR Switch Debouncer

todo

Use this library
Switch Debouncer - ISR debounce: Khoi Hoang https://github.com/khoih-prog/RPI_PICO_TimerInterrupt

ISR on megaATMEL arduinos will not let you use timers/delays inside ISRs. This means you will need to base your debouncing on other clock inputs, such as Airzone's bitshift clock cycle debouncer. The RP2040 have 16 ISR based timers, which are more accurate than software timers (eg millis()) and their execution is not blocked by poor function/code. This is great for mission critical stuff. In this case, we use them for the button debouncing.


```
#include "RPi_Pico_TimerInterrupt.h"


```




# I2C Connected Hardware (eg, Neopixel)

If you use an I2C connected device, eg Stemma QT, you need to access it via the I2C interface

todo
  
  
 ---------------
  
# Programming the ESC - BLHeli32Suite, BLHeliSSuite

todo
  
 
 ---------------
 
 Generic tutorials, files, samples for foam dart, nerf, etc
 
[ Home](README.md)
 


