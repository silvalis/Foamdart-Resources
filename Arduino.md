# Basic Arduino Tutorial
 
Arduinos are powerful and easily programmed devices that can enhance your foam dart blaster.

**Applications:**
 - Flywheelers 
 - HPA

**Functions**
 - Select fire
 - Brushless flywheels
 - Advanced fire control 
 

 
 
 
# Start here if you don't know what an Arduino is

## Arduino

It's a programmable microcontroller (MCU)

You will require both hardware and software

**Software**

(https://www.arduino.cc/)

This guide does not cover Arduino IDE setup, basic usage and language. Please go here for tutorials
(https://www.arduino.cc/en/Guide)

**Hardware**

Any Arduino Nano

![Fritzing pic of an Arduino Nano](/images/ArduinoNano-Fritzing.png)

Amazon
(https://www.amazon.com/Arduino-A000005-ARDUINO-Nano/dp/B0097AU5OU)

I like these low cost clones
Australia
(https://www.amazon.com.au/Arduino-Elegoo-ATmega328P-Without-Compatible/dp/B077272KMZ/)

## Basic Circuits

This is a basic button press LED circuit

### What are all these pins for?

An arduino has a number of pins connected to the MCU for a variety of uses. What you need to know are:

![Fritzing pic of an Arduino Nano](/images/ArduinoNano-Fritzing.png)

- Vin - Main power supply to the Arduino.
- GND - Ground
- 5V - 5VDC output supply. This can supply a max of about 500mA so do not attach motors and so on to it.
- D2 to D13 - Digital i/o pins. Configurable as an input or output. PWM on some pins. Use for on or off things like buttons, motors, etc
- A1 to A7 - Analogue i/o pins. Configurable as input or output. Use for things like battery monitors, adjustment potentiometers, etc
- The rest are not covered here

### Supply power to your Arduino

Arduino Nanos are 7-21VDC. 

Attach a power supply (eg batteries) to your Nano Vin and GND pins

!(/images/Battery.png)


### Input

Since we're programming nerf blasters, lets start with your basic requirement - an input switch that will cause the blaster to do something.
Your basic input circuit looks like this

![Pushbutton](/images/Pushbutton.png)

Note the following
- One side of the button is connected to D2
- The other side is connected to GND

This means that when you press the button, D2 is connected to GND and reports back as LOW. If it is not pushed, it reports HIGH

*** SIDEBAR: Button Addressing ***
>There are several methods of addressing Arduino pins from code, but the short version is:
>
>Digital D2-D13:  D2 = 2, D3 = 3, etc
>
>Analog A0-A7:    A0 = A0, A1 = A1, etc


There are two INPUT definitions - INPUT and INPUT_PULLUP. The short version for our purposes are, INPUT_PULLUP works better in high EMF noise environments. 

You can add this to your code with the following

```
pinMode(9, INPUT_PULLUP);
if (digitalRead(9) == LOW)
{
	// Do something
}
```




### Output

Define your output like this
`pinMode(3, OUTPUT);`

![LED](images/led.png)

If you set your output HIGH, it turns ON. LOW turns OFF.
Write to your output like this
`digitalWrite(3,HIGH);`


### Putting it together

Put the two together, and you get an LED turning on when the button is pressed

![circuit](/images/puttingittogether.png)

```
pinMode(9, INPUT_PULLUP);		// Input button
pinMode(3, OUTPUT);				// Output LED
if (digitalRead(9) == LOW)		// If button has been pressed
{
	digitalWrite(3,HIGH);		// Turn ON
}
else							// If button is not pressed
{
	digitalWrite(3,LOW);		// Turn OFF
}
```

### How does this relate to my flywheeler? I don't have LEDs!!

Swap out the LED with a circuit to drive the motors!

HOWEVER

The Arduino does not have the output voltage or current to drive DC motors. You need to use a power transistor circuit.

### Power MOSFET circuit

Go here for a complete guide on MOSFETs
(https://www.reddit.com/r/Nerf/comments/6ufmm8/the_complete_nerf_blaster_mosfet_wiring_tutorial/)

A transistor is essentially an electronically controlled switch - send a signal on the control pin to switch it ON and OFF.

You need:
- MOSFET (eg IRLB3034PBF)
- 10kohm Resistor
- Flyback Diode

Using the same inputs on the Arduino...

![mosfet wiring](/images/Mosfetcircuit.png)

You can also use premade modules, especially for lower power requirements. For example, for the pusher electronics, I like the Freetronics NDRIVE N-MOSFET module. This has a 20A max rating and is relatively compact, so easy to drop in.

(https://www.freetronics.com.au/products/n-mosfet-driver-output-module)

Program code for this is the same as for the LED circuit. All we have done is replaced the LED.


### Adding a pusher driver and a rev trigger

To add an automatic pusher to the circuit, I've simply duplicated the power transistor circuit and added another motor. Keep in mind that this will get you automatic only. This has been connected to an available digital pin, D4.

I have also added another button to D8. This could be used to trigger the pusher circuit.

![full circuit](/images/fullcircuit.png)

```
pinMode(9, INPUT_PULLUP);		// Input button
pinMode(8,
pinMode(3, OUTPUT);				// Output LED
if (digitalRead(9) == LOW)		// If button has been pressed
{
	digitalWrite(3,HIGH);		// Turn ON
}
else							// If button is not pressed
{
	digitalWrite(3,LOW);		// Turn OFF
}
```


## Troubleshooting

### My buttons aren't working

Electric motors, such as those used in Nerf flywheelers produce a fair bit of electromagnetic frequency noise (EMF). This, if close enough to your input wires, can induce current on them and mess with your input buttons and so on. Arduinos provide an internal "Pullup" resistor on its digital inputs which can block this from happening. Define your pin with INPUT_PULLUP instead.

`pinMode(9, INPUT_PULLUP);`

If its an extremely noisy environment, use an external resistor. These fare better when the switch is in close proximity to DC motors. 

10Kohm is a good resistance to start with. Noisier environments will require lower values.

![External pulluup](/images/pullupresistor.png)



## Basics covered









# Blaster Building

## Required Hardware
1. Arduino
2. Switches
3. Power Source
4. Wire
5. Flywheel Motors
6. Pusher Motor/Solenoid
7. MOSFET


## Blaster Configuration

## Flywheel Type: Brushed

This section covers control schemes and programming of your 

**Brushed Resources**

/r/Nerf MOSFET tutorial
(https://www.reddit.com/r/Nerf/comments/6ufmm8/the_complete_nerf_blaster_mosfet_wiring_tutorial/?utm_medium=android_app&utm_source=share)


## Flywheel Type: Brushless

**Brushless Resources**

Ultrasonic (https://www.thingiverse.com/thing:4266387)
Making Stuff Awesome group (https://www.facebook.com/groups/MakingStuffAwesome/)


## Pusher Type: Solenoid
Example: FTW Hyperdrive
<insert picture here>

**Requirements:**
 - Minimum 3S Lipo

**Pros:**
 - Easy to code
 - Limited circuitry required
 - Fun tactile response
 - Does not jam / Auto retract when power removed
 - Quiet
 - Faster first shot response

**Cons:**
 - Higher cost than motorised
 - Maximum ROF limited to <20DPS for most options

Solenoids are an extremely easy way of gaining electronic fire control. Without an MCU, they can simply be wired in series with a battery and switch for a nice tactile clunk when firing. Under MCU control, you can expect to gain around 10dps on 3S and up to 18 on 4S. They are very easy to program for and only need a basic power transistor circuit to operate. 

**Solenoid Recommendation**

My preference for electronic solenoids are the FTW Hyperdrive.
https://flywheeltheworld.com/shop/ftw-hyperdrive-solenoid/
These are a great solenoid, capable of both 3S and 4S and used to come with an extra return spring for higher rof. 

**Required Circuitry**

A standard power MOSFET is suitable - most solenoids of this size only require 8-12A at our 3S/4S voltages. There are several options here.

1. Narfduino
If you bought a Narfduino, you can simply connect the solenoid to the DRIVE pins on the Half-H. The full circuit is not required (STOP is not used).

2. Drop in module
There are plenty of premade modules available. One of my favourite is the Freetronics NDRIVE N-MOSFET module. This module is rated for 20A.
(https://www.freetronics.com.au/products/n-mosfet-driver-output-module)

3. Discrete Components
You can re-use the same MOSFET setup used in most high powered flywheeler circuits. These are often available as kits from foam dart shops, [such as this one from Blastertech](https://www.blastertech.com.au/Mosfet_High_Current_Kit/p2928293_18274115.aspx)

**Programming**

Solenoids are easy to program for - to extend the pusher, you energise the circuit. To retract the pusher, you simply remove power. In the arduino environment, this is simply turning the connected pin ON and OFF and also means in order to achieve SEMI/BURST/AUTO, it's as simple as counting the number of darts you want to shoot. 
I suggest a code block similar to this
```
// Pseudocode for a Select fire solenoid

extendtime = 75ms              // These need to be tweaked for the build in question, but simply 75ms to extend, 75ms to retract = a bit over 150ms per shot
retracttime = 75ms
burst = 3                      // Shoot 3 times in burst mode
shotcount = 0

IF (SEMI) THEN
  Fire()

ELSE IF (BURST)
  WHILE shotcount < burst
    Fire()
  ENDWHILE
  
ELSE                           // AUTO 
  WHILE Trigger == ON
    Fire()
  ENDWHILE


// Turn the solenoid on/off
function Fire() 
  Solenoid ON
  DELAY extendtime 
  Solenoid OFF
  DELAY retracttime

```
This can obviously be optimised.


## Pusher Type: Motorised
Example: Rapidstrike Circuit
<insert picture here>

**Requirements:**
 - Pusher parts (commercial or printed)
 - Pusher motor

**Pros:**
 - Cheap
 - Very High ROF
 - Fun tactile response
 - Widely available

**Cons:**
 - Noisy
 - Can be difficult to control
 - Complexity / High number of moving parts

Electronic control over these auto kits will give you excellent variable motor control and braking performance. You can achieve extremely high ROF using motorised pushers, however its a trade off for limited shot control. Some kits, such as the stryfe auto kit or the Woozi pusher box, have a built in bump switch to attempt to halt the pusher motor in a retracted state. These can be used for shot control.

In order to implement a motorised pusher, you need a Half-H Bridge circuit. These can be found as a Shield module, a single chip IC or built out of discrete components. Compared to the classic circuit found in a rapidstrike, we replace the switches with transistors, letting us drive far more powerful motors and even variable pusher rof.

> **What is a Half-H Bridge?**
> A H Bridge is a switching circuit that allows a DC motor to be run in both forward and reverse. 
> A Half-H Bridge has half of this functionality, allowing excellent motor control in a single direction.

The Narfduino has a major advantage here compared to the bare Nano - it has a built in Half-H. I would highly recommend, if available, the Narfduino boards are preferenced here due to the convenience and cost advantage. Airzone also has a micro Half-H bridge drop in module.
(https://blastersbyairzone.com/product/half-h-bridge-board/)

[Here is an excellent example of a discrete Half-H](https://hackaday.com/2011/10/28/motor-drivers-half-h-bridge-with-brake-and-more/)
!(https://hackaday.com/wp-content/uploads/2011/10/half-h-bridge-with-brake.jpg)


**Programming**

In general, motorised pushers are harder to program for if you do not add a multitude of pusher sensors. Burst is difficult to control and both semi/auto will be based on timing (eg time taken to push 1 dart). 

I suggest a code block similar to this
```
// Pseudocode for a Semi/auto pusher motor
semifiretime = 50ms // Arbitrary number plucked out of nowhere to describe how long it will take to start 1 shot

IF (SEMI) THEN
  Pusher ON
  delay semifiretime
  Pusher OFF					// then attempt to brake the motor

ELSE                            // AUTO 
  WHILE Trigger == ON
    Pusher ON
  ENDWHILE
  Pusher OFF

```
This can obviously be optimised.

If you add a sensor (eg a switch) that activates when the pusher is fully extended, or complete 1 cycle, you can then count the cycles and similar to the solenoid, implement burst.


## Fire Control: Semi
Not covered. These are almost all mechanical systems.

## Fire Control: Motorised Auto



## Fire Control: Electronic Semi/Auto/Other


## HPA

**Solenoid**

For Spexbz and Supercore builds that use a QEV, you need a 3 way 2 position solenoid. 

There are two solenoid choices that come in a reasonable size

- Airtac 3V110-06, and clones
- MAC 3 Port Solenoid 35A-ACA-DDBA-1BA

Ensure you buy the 12V version so you can use a compact 3S lipo.

**Programming**

HPA Solenoid programming is much like a large solenoid pusher. You will be limited by your fittings diameter / How quickly does the spexbq/supercore recharge. You can install bump switches to detect pusher position, electronic pressure sensors to report back to the MCU, or just trial and error to determine timing.


I suggest a code block similar to this
```
// Pseudocode for a Semi/auto pusher motor
semifiretime = 50ms // Arbitrary number plucked out of nowhere to describe how long it will take to start 1 shot

IF (SEMI) THEN
  Pusher ON
  delay semifiretime
  Pusher OFF					// then attempt to brake the motor

ELSE                            // AUTO 
  WHILE Trigger == ON
    Pusher ON
  ENDWHILE
  Pusher OFF

```
This can obviously be optimised.




## General Considerations

### Arduino Choice
I recommend 2 boards

1. Arduino Nano (and clone devices)

**Pros**
 - Widely available
 - Cheap
 - 7-21VDC input (eg, 2S-5S without a regulator)
 - Common Atmega 328p MCU, plenty of libraries, tutorials, support
 
**Cons**
 - Limited GND and 5VDC pins - DIY your own
 
2. [Airzone Narfduino](https://blastersbyairzone.com/product/narfduino/)

**Pros**
 - Similar to the Nano
 - Built in Half-H Bridge
 - A number of GND and 5VDC pins available on the board
 
 **Cons**
 - Costs more than a nano
 
 Other devices have more cons, for example
 - UNO is quite large
 - Micro has a 7-12VDC input - most likely requires additional power supply circuitry
 - Nano Every has a newer MCU and is more powerful. Fine, but requires slightly different code handling

### Power Supply
Power supply choice depends on your application.

**Brushed Flywheel**

2S or 3S lipo, direct battery connection

**Brushless Flywheel**

2S-5S fine, 6S will require a regulator for the arduino.

