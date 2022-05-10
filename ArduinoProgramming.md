
# Programming an arduino properly

There are a number of things you need to do to run your arduino more efficiently

[Return to the start](Arduino.md)
[The next section covers hardware options when building your blaster](BlasterTypes.md)

## Better Button Handling

Buttons should be polled once per cycle, to ensure that the inputs are checked on a regular basis rather than read adhoc. This is increasingly more important as your program grows larger. 

Poll the status of the buttons once per cycle and place it in your loop. 

```
#define PIN_TRIGGER_FIRE 8
#define PIN_TRIGGER_REV 9

byte FireButtonState = HIGH;
byte RevButtonState = HIGH;


void loop()
{
	FireButtonState = digitalRead(PIN_TRIGGER_FIRE);
	RevButtonState = digitalRead(PIN_TRIGGER_FIRE);
}


```

Question: What is the issue with this code?

Answer: It doesn't detect fleeting button presses

This can be solved using Interrupts.
This is out of scope of this tutorial (for the moment)

Button debouncing is also out of scope of this tutorial. 

## Delays to ensure things happen

In the case of a solenoid, it's similar to a binary switch - ON is foward, OFF is retracted. This means you need to time how long its on and off so that you don't:
1) unnecessarily hold the solenoid ON (uses battery power)
2) shortstroke the solenoid. Energise or deenergise before it has reached fully extended or fully retracted

`delay(x)`
where x is the time in milliseconds.

You can do something like this

```		
digitalWrite(4,HIGH);		// Start pusher
delay (100);			// energise for 100ms
digitalWrite(3,LOW);		// Turn OFF pusher
delay (100);			// denergise for 100ms
```




## Full Code

Using the previous example with the same circuit

![full circuit](/images/fullcircuit.png)

```
// INPUTS
#define PIN_TRIGGER_FIRE 8
#define PIN_TRIGGER_REV 9
// OUTPUTS
#define PIN_MOTOR_REV 3
#define PIN_MOTOR_PUSH 4


void setup()
{
	pinMode(PIN_TRIGGER_REV, INPUT_PULLUP);		// REV button
	pinMode(PIN_TRIGGER_FIRE, INPUT_PULLUP);	// PUSH button
	pinMode(PIN_MOTOR_REV, OUTPUT);			// Output REV wheels
	pinMode(PIN_MOTOR_PUSH, OUTPUT);		// Output PUSH motor

}

void loop()
{

	if (digitalRead(9) == LOW)		// If button has been pressed
	{
		digitalWrite(3,HIGH);		// Turn ON
		if (digitalRead(8) == LOW)	// If the rev is pressed, then we can press pusher)
		{
			digitalWrite(4,HIGH);	// Start pusher
			delay (100);		// energise for 100ms
			digitalWrite(3,LOW);	// Turn OFF pusher
			delay (100);		// denergise for 100ms
		}
	}
	else					// If REV button is not pressed, stop both
	{
		digitalWrite(3,LOW);		// Turn OFF rev
	}

}

```




## Programming covered
[Return to the start](Arduino.md)

[The next section covers hardware options when building your blaster](BlasterTypes.md)