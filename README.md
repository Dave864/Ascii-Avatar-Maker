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

####Source Files
