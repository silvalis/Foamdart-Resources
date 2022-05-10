# Page links

[Introduction](Arduino.md)

[Programming](ArduinoProgramming.md)

# Blaster Building

## Required Hardware
1. Arduino
2. Switches
3. Power Source
4. Wire
5. Flywheel Motors
6. Pusher Motor/Solenoid
7. MOSFET(s)


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


## Done

[Return to the start](Arduino.md)

[Back to Programming](ArduinoProgramming.md)