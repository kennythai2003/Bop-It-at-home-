# Bop It! (at home)  
*by Kenny Thai*  
*June 12, 2024*

[Video Demo](https://www.youtube.com/watch?v=Cod0daSMnFg&ab_channel=KennyThai)

## Introduction  
This project replicates the classic game **"Bop It!"** with some modifications. The game outputs a command, and the player must respond correctly within a set time limit. If successful, the next command is prompted; otherwise, the game resets, and the score is displayed. The objective is to complete as many commands as possible, with points earned for each successful action. The game increases in difficulty as the time limit shortens after each round.

## Build-Upons  
- **PIR Sensor**  
  - Used to detect motion (hover action)
- **DHT-11 Sensor**  
  - Used to detect temperature change (blow action)
- **IR Remote**  
  - Used to turn the game on/off

## User Guide  
To start the game, press the power button on the IR Remote. The LCD display will show a command, such as **press**, **up**, **down**, **left**, **right**, **blow**, or **hover**.  
- If the correct action is performed, the score increases, and the next command is displayed.
- If the wrong action is performed, the final score is shown, and the player can restart by pressing the power button.  
The game becomes progressively harder as the time limit for each command decreases.

## Hardware Components Used  
- PIR Sensor
- DHT-11 Sensor
- IR Remote
- Joystick
- LCD Display

## Software Libraries Used  
- **irAVR.h**  
  Used for setting up and operating the IR Remote.
- **LCD.h**  
  Used for controlling the LCD display.
- **periph.h**  
  Used for `ADC_read()` to handle joystick inputs.
- **serialATmega.h**  
  Mainly used for debugging with the `serial_println()` function.
- **timerISR.h**  
  Sets up the concurrent state machines.


