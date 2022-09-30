Build Guide for MK3 Flycore

Required:

STLs
----- ----- -----
Brushless Flycore & associated files


Electronics
----- ----- -----
USB Linker or Flight Controller required for BLHeli_32 programming

ESC:          (1) Mamba F40_128K 4 in 1 ESC
Motors:       (2) EMAX ECO 1407 4100KV
MCU:          (1) Seeed Xiao RP2040
Supply:       (1) Pololu 2843 5V, 500mA Vreg
Pusher Motor: (1) N20 12VDC 1000rpm
DC Driver:    (1) Pololu 4036 DRV8876 Motor Driver

Electronics Parts
----- ----- -----
Switches:     (3) Cherry DC2 or clone microswitch
Selector Sw:  (1) As required
Battery Cntr: (1) Your choice
Wiring:       Battery Input           15cm Red/Black 18AWG Silicone Wire
              Motor Driver Supply     25cm Red/Black 22AWG Silicone Wire
              Vreg                    10cm Red/Black/Blue 22AWG Silicone Wire
              ESC Signalling          5cm 2 wire colours 28AWG
              Motor Driver Signalling 25cm 3 wire colours 28AWG
              Switches                25cm 4 wire colours 28AWG
              
              
Steps
----- ----- -----
0- Start printing now

1- ESC Programming
Download BLHeliSuite32
Solder a battery lead to the ESC
Connect ESC to USB Linker or FC
Update BLHeli firmware to latest and use ultrasonic programming guide for settings
Remove battery lead

2- Wiring Diagram goes here


3- Arduino program goes here



              
