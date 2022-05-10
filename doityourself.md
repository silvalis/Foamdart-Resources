
# Page links

[Introduction](Arduino.md)

[Programming](ArduinoProgramming.md)

[BlasterTypes](BlasterTypes.md)

# Time to do it yourself

## Step 1

Choose your options

### Drive

1) Brushed 		>> MOSFET
2) Brushless 	>> ESCs
3) HPA 			>> Nil


### Pusher

1) Mechanical	 >> This is an arduino tutorial
2) Motorised Auto Only 		>> Add a MOSFET
3) Motorised w/ Brake		>> Add a Half-H
4) Solenoid					>> Add a MOSFET


### Switches

1) Trigger (Combined)				>> +1 Switch
2) Rev Trigger (Trigger/Rev)		>> +1 Switch
3) Select Fire (+1 switch for each mode) >> +1 Switch per mode


### Others

1) Battery volt meter
2) OLED display


## Step 2

### Drive


### Switches
Count how many switches you need and directly attach them to any available digital pin.

This example has 2 switches
(images/2button.png)


### Others
See airzonesama's resources

## Step 3

Write a program

1) Define your switch inputs & pins. What do they do?
2) Define your outputs. What drives what?
3) Link your input to your output. Which switch will trigger which output?