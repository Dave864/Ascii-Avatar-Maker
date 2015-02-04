##Ascii Avatar Maker

####Description
This system allows a person to create a custom ascii character on a LED matrix, using
buttons and a joystick, and then display that character on an LCD screen.

####User Guide

######Board Orientation
The description of the controls assume that the board is oriented in this manner
- The LCD screen is at the top
- The LED display and the buttons are at the bottom
- The joystick is on the right

######Controls and Interface
- Interface
  - The LED display is where the pattern for the ascii character will be created
    - The space that can be worked in is indicated by orange columns
    - The current character is displayed in green
    - There is a red blinking cursor indicating which pixel will be edited
 - The LCD screen is where the custom ascii character will be displayed when 
completed
 - The buttons are used to erase and color pixels as well as load the current pattern
to the LCD screen
 - The joystick moves a cursor across the LED display
- Controls
 - Left Button: Colors the pixel under the cursor
 - Middle Button: Erases the pixel under the cursor
 - Right Button: Loads the current pattern onto the LCD screen
 - Joystick: Moves the cursor around the LED interface

[Video Demo]: https://www.youtube.com/watch?v=VSpaqil_ayc/ "Video Demo"

######Bugs
- Pressing the color and erase buttons simultaneuosly will not affect the pixel under
the cursor
- The ascii pattern can be edited while the load button is being held, but the new edits will not be implemented until the load button is released and pressed again

####Technologies and Components Used
- AVR Studio 6.2
- 1588ABEG LED Matrix
- Parallax 2-axis Joystick 27800
- ATmega 1284
- CD74HC595 8-bit Shift Register
- LCM-s01602DTR/M 16x2 character LCD module
- 10k potentiometer
- KS-03Q-01 push button
