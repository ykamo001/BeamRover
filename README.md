# BeamRover
An embedded systems car that operates on light detection

## High Level Description:
Beam Rover is an impersonation of the famous Mars Rover with some modifications. Beam Rover utilizes Light Dependent Resistors (LDRs) to see and gain information from it’s surroundings. Furthermore, it also uses a keypad to take in user input on the desires functionality. There are 3 modes: normal, night, and manual. In normal mode, the Beam Rover will proceed to the brightest location it can find, and will go in the direction that has the most light. In contrast, night mode is the opposite, in that it will try to find the darkest area possible and stay away from any light it finds. In manual mode, the user can program in a specific path that it wants the rover to follow and will execute it on command.

## Demo Video:
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/9hwpurE0pgE/maxresdefault.jpg)](https://youtu.be/9hwpurE0pgE)

## User Guide:
Use keypad to enter in commands, and LCD to view directions/output. Entering `*` at anytime on the keypad will exit the current functionality of the rover and take you back to the menu. Upon starting up the rover, the LDRs are configured, in that it will take the readings from when it first starts as the base level of light that it will be working with as a constant throughout it’s duration. When in menu, pressing `*` takes you to an options list of modes, wherein option `A` is normal, `B` is night, and `C` is manual. Pressing one of these options puts the rover into that specified mode, and will continue to operate in that mode until exited, which can be done by pressing `*` as mentioned before.

## Technologies Used: 
* 2 Atmega1284 microcontrollers
* 4 Light Dependent Resistors (LDRs)
* 3 Light-Emitting Diodes (LEDs)
* 1 Liquid-Crystal Display (LCD)
* 1 4-battery 12-V power source
* 2 DC motors
* Breadboard
* 1 L293d motor driver
* 1 10-pin switch
* Atmel Studios
* Electrical Wires
* AVR
* 7 330-ohm Resistors
* 1 Potentiometer

## Source Files and Description:
1. [BeamRoverMaster.c](https://github.com/ykamo001/BeamRover/blob/master/Source%20Files/BeamRoverMaster.c): The source file for the master microcontroller. Handles all the inputs from the keypad and sensor readings, and sends information to the slave microcontroller through SPI. Does all the calculations and filtering involved with the ADC readings from the LDRs and Keypad inputs. Also outputs which sensor currently has the highest reading.
2. [BeamRoverServant.c](https://github.com/ykamo001/BeamRover/blob/master/Source%20Files/BeamRoverServant.c): Source file for the slave microcontroller. Handles all the output for the DC motors and LCD screen. Receives data from the master microcontroller via SPI, and according to the information that is sent, outputs the proper bits to the specific ports and pins. Also handles PWM for the DC motors.

## Wiring Setup:
###LCD
![alt text](https://github.com/ykamo001/BeamRover/blob/master/Circuitry/LCD.png)

### Motor Driver
![alt text](https://github.com/ykamo001/BeamRover/blob/master/Circuitry/L293d%20Motor%20Driver.png)

### Slave Microcontroller
![alt text](https://github.com/ykamo001/BeamRover/blob/master/Circuitry/Slave%20Microcontroller.JPG)

### Master Microcontroller
![alt text](https://github.com/ykamo001/BeamRover/blob/master/Circuitry/Master%20Microcontroller.JPG)
