# Foamdart Resources
 
 
 ---------------
 If you like my work, please buy me a coffee!
 https://www.buymeacoffee.com/silvalisdesigns
 ---------------
 
 To do:
 [ Raspberry Pico RP2040](RP2040Programming.md)
 
 RP2040 programming tutorial
 
 The RP2040 is a fantastic little mcu that has Programmable I/O (PIO) modules. These run independently of the main process and as a result does not consume clock cycles. 
 
 For our purposes, this means that we can implement button debouncing code and button interrupt code in PIO and not cause the main process to pause/wait while we look for the input to settle.
 
 The RP2040 also has hardware PWM on each pin, cost is about 1/2 to 2/3 that of a nano and did I mention that it's tiny? It's half the size of the arduino nano.
 
 The downside is that it's a 3V3 MCU, although accepts 5V Vin. This means you may need to level shift down to 3V3. For my projects, I added a minature 5VDC Vreg to step down batt volts from 3s or 4s down to 5V. Why did I pick 5V instead of a 3V3 Vreg? So I could use the onboard 3V3 pin as a Vsource for other logic inputs.
 
 
 ---------------
 
 Generic tutorials, files, samples for foam dart, nerf, etc
 
[ Start here if you don't know what you're doing](Arduino.md)
 


