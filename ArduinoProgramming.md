
# Programming an arduino properly

Arduinos are constantly looping through its code and operate on a state machine behavioural system. Code examples given previously were simplied for the tutorial

A state machine is a behavioural machine that has a finite number of states and a given input into the machine performs a transition and produces an output. A simple way of explaining this is:

![finite state machine](/images/fsm.png)

When you press the button, the flywheel motors start. When you release it, the motors stop.

###So whats the point of this?

You need to organise your code properly.

Arduino code contains 4 main areas:

***Declarations***

Where you declare your pins, variables, constants, etc

***setup()***

Where you place run-once-on-startup commands, such as initial values

***loop()***

Code that is executed every cycle

***user functions*** 

User declared functions

## Polling buttons

Buttons should be polled once per cycle, to ensure that the inputs are checked on a regular basis rather than read adhoc. This is increasingly more important as your program grows larger. 

Create a function 

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
	pinMode(PIN_TRIGGER_FIRE, INPUT_PULLUP);		// PUSH button
	pinMode(PIN_MOTOR_REV, OUTPUT);			// Output REV wheels
	pinMode(PIN_MOTOR_PUSH, OUTPUT);			// Output PUSH motor

}

void loop()
{

	if (digitalRead(9) == LOW)		// If button has been pressed
	{
		digitalWrite(3,HIGH);		// Turn ON
		if (digitalRead(8) == LOW)		// If the rev is pressed, then we can press pusher)
		{
			digitalWrite(4,HIGH);		// Start pusher
		}
	}
	else					// If REV button is not pressed, stop both
	{
		digitalWrite(4,LOW);		// Turn OFF pusher
		digitalWrite(3,LOW);		// Turn OFF rev
	}

}

```




## Programming covered

The next section covers hardware options when building your blaster