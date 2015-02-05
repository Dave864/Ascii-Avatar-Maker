/*
 * Ascii_Avatar_Maker.c
 *
 * Created: 5/23/2014 4:04:59 PM
 *  Author: Owner
 */

#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "io.c"

typedef unsigned char u_char;
typedef unsigned long int u_lint;

char * message1 = "Current Avatar";
char * message2 = "Loading new";
char * message3 = "Avatar";
char * message4 = "...Done!";
//the bit pattern for the ascii avatar
u_char avatar[] = {0x04, 0x0E, 0x1F, 0x04, 0x04, 0xAA, 0xAA, 0x00};
//cursor position
u_char cursor[] = {2, 4};
//flag for lighting  the cursor
u_char cursor_on;
//flag for blinking the cursor
u_char cursor_blink;
//values that store the input from the joystick
int16_t U_D;
int16_t L_R;

//prototypes
int TickFct_DisplayMatrixRow(int state);
int TickFct_DisplayMatrixColumn_R(int state);
int TickFct_DisplayMatrixColumn_G(int state);
int TickFct_CursorOn(int state);
int TickFct_CursorPos_H(int state);
int TickFct_CursorPos_V(int state);
int TickFct_EditAvatar(int state);
int TickFct_DisplayAvatar(int state);
int TickFct_LoadAvatar(int state);

//Initialize the ADC
void ADC_init()
{
	ADMUX = (1<<REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

//Read from the ADC channel ch
uint16_t ReadADC(uint8_t ch)
{	
	//Select ADC Channel
	ch &= 0x07;
	ADMUX = (ADMUX & 0xF8) | ch;

	//Start Single conversion
	ADCSRA |= (1 << ADSC);

	//Wait for conversion to complete
	while(ADCSRA & (1 << ADSC));

	//Clear ADIF
	ADCSRA |= (1<<ADIF);
	return (ADC);
}

int main(void)
{
	DDRA = 0x83; PORTA = 0x7C;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	//task periods
	u_lint DMRTick_calc = 3;
	u_lint DMCTick_R_calc = 3;
	u_lint DMCTick_G_calc = 3;
	u_lint CursorPos_H_calc = 200;
	u_lint CursorPos_V_calc = 200;
	u_lint CursorOn_calc = 500;
	u_lint EditAvatar_calc = 100;
	u_lint DisplayAvatar_calc = 100;
	
	//calculate GDC
	u_lint tmpGCD = 1;
	tmpGCD = findGCD(DMCTick_R_calc, DMRTick_calc);
	tmpGCD = findGCD(tmpGCD, DMCTick_G_calc);
	tmpGCD = findGCD(tmpGCD, CursorPos_H_calc);
	tmpGCD = findGCD(tmpGCD, CursorPos_V_calc);
	tmpGCD = findGCD(tmpGCD, CursorOn_calc);
	tmpGCD = findGCD(tmpGCD, EditAvatar_calc);
	tmpGCD = findGCD(tmpGCD, DisplayAvatar_calc);
	
	u_lint GCD = tmpGCD;
	
	//recalculate periods
	u_lint DMRTick_period = DMRTick_calc/GCD;
	u_lint DMCTick_R_period = DMCTick_R_calc/GCD;
	u_lint DMCTick_G_period = DMCTick_G_calc/GCD;
	u_lint CursorPos_H_period = CursorPos_H_calc/GCD;
	u_lint CursorPos_V_period = CursorPos_V_calc/GCD;
	u_lint CursorOn_period = CursorOn_calc/GCD;
	u_lint EditAvatar_period = EditAvatar_calc/GCD;
	u_lint DisplayAvatar_period = DisplayAvatar_calc/GCD;
	
	//task array declaration
	static task taskDMR, taskDMC_R, taskDMC_G, taskCursor_Pos_H,
				taskCursor_Pos_V, taskCursor_On, taskEditAvatar,
				taskDisplayAvatar;
	task *tasks[] = { &taskDMR, &taskDMC_R, &taskDMC_G, &taskCursor_Pos_H,
					&taskCursor_On, &taskCursor_Pos_V, &taskEditAvatar,
					&taskDisplayAvatar };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	//Task DMR
	taskDMR.state = -1;
	taskDMR.period = DMRTick_period;
	taskDMR.elapsedTime = DMRTick_period;
	taskDMR.TickFct = &TickFct_DisplayMatrixRow;
	
	//Task DMC_R
	taskDMC_R.state = -1;
	taskDMC_R.period = DMCTick_R_period;
	taskDMC_R.elapsedTime = DMCTick_R_period;
	taskDMC_R.TickFct = &TickFct_DisplayMatrixColumn_R;
	
	//Task DMC_G
	taskDMC_G.state = -1;
	taskDMC_G.period = DMCTick_G_period;
	taskDMC_G.elapsedTime = DMCTick_G_period;
	taskDMC_G.TickFct = &TickFct_DisplayMatrixColumn_G;
	
	//Task Cursor_Pos_H
	taskCursor_Pos_H.state = -1;
	taskCursor_Pos_H.period = CursorPos_H_period;
	taskCursor_Pos_H.elapsedTime = CursorPos_H_period;
	taskCursor_Pos_H.TickFct = &TickFct_CursorPos_H;
	
	//Task Cursor_Pos_V
	taskCursor_Pos_V.state = -1;
	taskCursor_Pos_V.period = CursorPos_V_period;
	taskCursor_Pos_V.elapsedTime = CursorPos_V_period;
	taskCursor_Pos_V.TickFct = &TickFct_CursorPos_V;
	
	//Task Cursor_On
	taskCursor_On.state = -1;
	taskCursor_On.period = CursorOn_period;
	taskCursor_On.elapsedTime = CursorOn_period;
	taskCursor_On.TickFct = &TickFct_CursorOn;
	
	//Task EditAvatar
	taskEditAvatar.state = -1;
	taskEditAvatar.period = EditAvatar_period;
	taskEditAvatar.elapsedTime = EditAvatar_period;
	taskEditAvatar.TickFct = &TickFct_EditAvatar;
	
	//Task DisplayAvatar
	taskDisplayAvatar.state = -1;
	taskDisplayAvatar.period = DisplayAvatar_period;
	taskDisplayAvatar.elapsedTime = DisplayAvatar_period;
	taskDisplayAvatar.TickFct = &TickFct_DisplayAvatar;
	
	//Set Timer
	TimerSet(GCD);
	TimerOn();
	
	ADC_init();
	LCD_init();
	
	unsigned short i;
    while(1)
    {
        for ( i = 0; i < numTasks; i++ ) {
	        // Task is ready to tick
	        if ( tasks[i]->elapsedTime == tasks[i]->period ) {
		        // Setting next state for task
		        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
		        // Reset the elapsed time for next tick.
		        tasks[i]->elapsedTime = 0;
	        }
	        tasks[i]->elapsedTime += 1;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
}

//handle up-down movement
void moveU_D(int16_t U_D)
{
	u_char tmp = cursor[1];
	if(U_D > 0)
	{
		tmp = (tmp == 7) ? 0 : tmp+1;
	}
	else if(U_D < 0)
	{
		tmp = (tmp == 0) ? 7 : tmp-1;
	}
	cursor[1] = tmp;
}

//handle left-right movement
void moveL_R(int16_t L_R)
{
	u_char tmp = cursor[0];
	if(L_R < 0)
	{
		tmp = (tmp == 4) ? 0 : tmp+1;
	}
	else if(L_R > 0)
	{
		tmp = (tmp == 0) ? 4 : tmp-1;
	}
	cursor[0] = tmp;
}

//determines horizontal cursor movement from joystick input
enum CPH_States {CPH_Wait, CPH_Read, CPH_Stay, CPH_Move, CPH_Ack};
int TickFct_CursorPos_H(int state)
{
	L_R = ReadADC(3);
	L_R -= 512;
	switch(state)//Transitions
	{
		case CPH_Stay:
			if((L_R <= 250) && (L_R > -249))
			{
				state = CPH_Stay;
			}
			else
			{
				state = CPH_Move;
			}
			break;
		case CPH_Move:
			if((L_R <= 250) && (L_R > -249))
			{
				state = CPH_Stay;
			}
			else
			{
				state = CPH_Move;
			}
			break;
		default:
			state = CPH_Stay;
			break;
	}
	switch(state)//Actions
	{
		case CPH_Stay:
			cursor_blink = 0;
			break;
		case CPH_Move:
			moveL_R(L_R);
			cursor_blink = 1;
			cursor_on = 1;
			break;
	}
	return state;
}

//determines vertical cursor movement from joystick input
enum CPV_States {CPV_Stay, CPV_Move};
int TickFct_CursorPos_V(int state)
{
	U_D = ReadADC(2);
	U_D -= 512;
	switch(state)//Transitions
	{
		case CPV_Stay:
			if((U_D <= 250) && (U_D > -249))
			{
				state = CPV_Stay;
			}
			else
			{
				state = CPV_Move;
			}
			break;
		case CPV_Move:
			if((U_D <= 250) && (U_D > -249))
			{
				state = CPV_Stay;
			}
			else
			{
				state = CPV_Move;
			}
			break;
		default:
			state = CPV_Stay;
			break;
	}
	switch(state)//Actions
	{
		case CPV_Stay:
			cursor_blink = 0;
			break;
		case CPV_Move:
			moveU_D(U_D);
			cursor_blink = 1;
			cursor_on = 1;
			break;
	}
	return state;
}

//blinks the cursor on and off
enum CO_States {CO_On, CO_Off};
int TickFct_CursorOn(int state)
{
	switch(state)//Transitions
	{
		case CO_On:
			if(cursor_blink)
			{
				state = CO_On;
			}
			else
			{
				state = CO_Off;
			}
			break;
		case CO_Off:
			state = CO_On;
			break;
		default:
			state = CO_On;
			break;
	}
	switch(state)//Actions
	{
		case CO_On:
			cursor_on = 1;
			break;
		case CO_Off:
			cursor_on = 0;
			break;
	}
	return state;
}

//edits the avatar based on button inputs
void editAvatar(char erase)
{
	u_char rowNum = cursor[1];
	u_char row = avatar[rowNum];
	u_char led = 0x01;
	led = led << cursor[0];
	if(erase)
	{
		row &= (~led);
	}
	else
	{
		row |= led;
	}
	avatar[rowNum] = row;
}

//determines whether to erase or color LED at cursor position
enum EA_States {EA_NoEdit, EA_Erase, EA_Color};
int TickFct_EditAvatar(int state)
{
	u_char erase = ~PINA & 0x20;
	u_char color = ~PINA & 0x40;
	switch(state)//Transitions
	{
		case EA_NoEdit:
			if(erase && !color)
			{
				state = EA_Erase;
			}
			else if(color && !erase)
			{
				state = EA_Color;
			}
			else
			{
				state = EA_NoEdit;
			}
			break;
		case EA_Erase:
			if(erase && !color)
			{
				state = EA_Erase;
			}
			else if(color && !erase)
			{
				state = EA_Color;
			}
			else
			{
				state = EA_NoEdit;
			}
			break;
		case EA_Color:
			if(erase && !color)
			{
				state = EA_Erase;
			}
			else if(color && !erase)
			{
				state = EA_Color;
			}
			else
			{
				state = EA_NoEdit;
			}
			break;
		default:
			state = EA_NoEdit;
			break;
	}
	switch(state)//Actions
	{
		case EA_NoEdit:
			break;
		case EA_Erase:
			editAvatar(1);
			break;
		case EA_Color:
			editAvatar(0);
			break;
	}
	return state;
}

//Displays the last loaded custom avatar
void display()
{
	LCD_ClearScreen();
	u_char pos;
	for(pos = 2; pos < 16; ++pos)
	{
		LCD_Cursor(pos);
		LCD_WriteData(message1[pos-2]);
	}
	pos = 24;
	LCD_Cursor(pos);
	LCD_WriteData(0x08);
}

//Loads a new custom avatar
void load(u_char location)
{
	LCD_ClearScreen();
	u_char i;
	if(location < 8)
	{
		LCD_WriteCommand(0x40+(location*8));
		for(i = 0; i < 8; ++i)
		{
			LCD_WriteData(avatar[i]);
		}
	}
	for(i = 1; i < 12; ++i)
	{
		LCD_Cursor(i);
		LCD_WriteData(message2[i-1]);
	}
	for(i = 0; i < 6; ++i)
	{
		LCD_Cursor(i+17);
		LCD_WriteData(message3[i]);	
	}
}

void loadDone()
{
	u_char i;
	for(i = 0; i < 8; ++i)
	{
		LCD_Cursor(i+23);
		LCD_WriteData(message4[i]);
	}
}

//Displays and loads the custom avatar
enum DA_States {DA_Load, DA_Display, DA_MakeNew, DA_Wait_but};
int TickFct_DisplayAvatar(int state)
{
	u_char Load = ~PINA & 0x10;
	switch(state)//Transitions
	{
		case DA_Load:
			if(Load)
			{
				state = DA_MakeNew;
			}
			else
			{
				state = DA_Display;
			}
			break;
		case DA_Display:
			if(Load)
			{
				state = DA_MakeNew;
			}
			else
			{
				state = DA_Display;
			}
			break;
		case DA_MakeNew:
			state = DA_Wait_but;
			loadDone();
			break;
		case DA_Wait_but:
			if(Load)
			{
				state = DA_Wait_but;
			}
			else
			{
				state = DA_Load;
			}
			break;
		default:
			state = DA_Load;
			break;
	}
	switch(state)//Actions
	{
		case DA_Load:
			display();
			break;
		case DA_Display:
			break;
		case DA_MakeNew:
			load(0);
			break;
		case DA_Wait_but:
			break;
	}
	return state;
}

//converts 5 bit data to corresponding 8 bit bordered format
unsigned char createColumn(u_char data){
	return ~data;
}

//sets data at x position labeled by cursor
unsigned char setCursor(u_char data){
	u_char tmp = 0x01;
	tmp = tmp << cursor[0];
	tmp = data & ~tmp;
	return tmp;
}

//Turns on a row given by data
void transmit_data_row(u_char data){
	int i;
	for(i = 0; i < 8; ++i){
		//sets SRCLR to 1
		//clears SRCLK
		PORTC = (PORTC & 0xF0) | 0x8;
		//sets SER = next data bit to be sent
		PORTC |= ((data >> i) & 0x01);
		//sets SRCLK = 1, shifting next data bit into shift reg
		PORTC |= 0x04;
	}
	//set RCLK = 1, copying data from "Shift" into "Storage" reg
	PORTC |= 0x02;
	//clears all lines for new transmission
	PORTC = (PORTC & 0xF0) | 0x00;
}

//Lights up LED matrix rows
enum DMR_States {DMR_R1, DMR_R2, DMR_R3, DMR_R4, DMR_R5, DMR_R6, DMR_R7, DMR_R8};
int TickFct_DisplayMatrixRow(int state){
	switch(state)//Transitions
	{
		case DMR_R1:
			state = DMR_R2;
			break;
		case DMR_R2:
			state = DMR_R3;
			break;
		case DMR_R3:
			state = DMR_R4;
			break;
		case DMR_R4:
			state = DMR_R5;
			break;
		case DMR_R5:
			state = DMR_R6;
			break;
		case DMR_R6:
			state = DMR_R7;
			break;
		case DMR_R7:
			state = DMR_R8;
			break;
		case DMR_R8:
			state = DMR_R1;
			break;
		default:
			state = DMR_R1;
			break;
	}
	switch(state)//Actions
	{
		case DMR_R1:
			transmit_data_row(0x01);
			break;
		case DMR_R2:
			transmit_data_row(0x02);
			break;
		case DMR_R3:
			transmit_data_row(0x04);
			break;
		case DMR_R4:
			transmit_data_row(0x08);
			break;
		case DMR_R5:
			transmit_data_row(0x80);
			break;
		case DMR_R6:
			transmit_data_row(0x40);
			break;
		case DMR_R7:
			transmit_data_row(0x20);
			break;
		case DMR_R8:
			transmit_data_row(0x10);
			break;
	}
	return state;
}

//Lights up the LED at cursor position red
void transmit_data_column_R(u_char data){
	if(data == 0xFF)
	{
		PORTA = (PORTA & 0x7F) | 0x80;
		PORTC = (PORTC & 0x0F) | 0xF0;
	}
	else
	{
		u_char tmp = 0x01;
		tmp = tmp << data;
		if(tmp == 0x10)
		{
			PORTA = (PORTA & 0x7F);
			PORTC = (PORTC & 0xFF);
		}
		else if(tmp == 0x08)
		{
			PORTA = (PORTA & 0xFF);
			PORTC = (PORTC & 0xEF);		
		}
		else if(tmp == 0x04)
		{
			PORTA = (PORTA & 0xFF);
			PORTC = (PORTC & 0xDF);
		}
		else if(tmp == 0x02)
		{
			PORTA = (PORTA & 0xFF);
			PORTC = (PORTC & 0xBF);
		}
		else
		{
			PORTA = (PORTA & 0xFF);
			PORTC = (PORTC & 0x7F);
		}
	}
}

//Lights up the columns the color red
enum DMC_R_states {DMC_R_R1, DMC_R_R2, DMC_R_R3, DMC_R_R4, DMC_R_R5, DMC_R_R6, DMC_R_R7, DMC_R_R8};
int TickFct_DisplayMatrixColumn_R(int state){
	switch(state)//Transitions
	{
		case DMC_R_R1:
			state = DMC_R_R2;
			break;
		case DMC_R_R2:
			state = DMC_R_R3;
			break;
		case DMC_R_R3:
			state = DMC_R_R4;
			break;
		case DMC_R_R4:
			state = DMC_R_R5;
			break;
		case DMC_R_R5:
			state = DMC_R_R6;
			break;
		case DMC_R_R6:
			state = DMC_R_R7;
			break;
		case DMC_R_R7:
			state = DMC_R_R8;
			break;
		case DMC_R_R8:
			state = DMC_R_R1;
			break;
		default:
			state = DMC_R_R1;
			break;
	}
	switch(state)//Actions
	{
		case DMC_R_R1:
			if((cursor[1] == 0) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R2:
			if((cursor[1] == 1) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R3:
			if((cursor[1] == 2) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R4:
			if((cursor[1] == 3) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R5:
			if((cursor[1] == 4) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R6:
			if((cursor[1] == 5) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R7:
			if((cursor[1] == 6) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
		case DMC_R_R8:
			if((cursor[1] == 7) && cursor_on)
			{
				transmit_data_column_R(cursor[0]);
			}
			else
			{
				transmit_data_column_R(0xFF);
			}
			break;
	}
	return state;
}

void transmit_data_column_G(u_char data){
	PORTB = (PORTB & 0xE0) | data;
}

//Lights up the columns the color green
enum DMC_G_states {DMC_G_R1, DMC_G_R2, DMC_G_R3, DMC_G_R4, DMC_G_R5, DMC_G_R6, DMC_G_R7, DMC_G_R8};
int TickFct_DisplayMatrixColumn_G(int state){
	switch (state)//Transitions
	{
		case DMC_G_R1:
			state = DMC_G_R2;
			break;
		case DMC_G_R2:
			state = DMC_G_R3;
			break;
		case DMC_G_R3:
			state = DMC_G_R4;
			break;
		case DMC_G_R4:
			state = DMC_G_R5;
			break;
		case DMC_G_R5:
			state = DMC_G_R6;
			break;
		case DMC_G_R6:
			state = DMC_G_R7;
			break;
		case DMC_G_R7:
			state = DMC_G_R8;
			break;
		case DMC_G_R8:
			state = DMC_G_R1;
			break;
		default:
			state = DMC_G_R1;
			break;
	}
	unsigned char tmp;
	switch (state)//Actions
	{
		case DMC_G_R1:
			tmp = avatar[0];
			if(cursor[1] == 0)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R2:
			tmp = avatar[1];
			if(cursor[1] == 1)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R3:
			tmp = avatar[2];
			if(cursor[1] == 2)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R4:
			tmp = avatar[3];
			if(cursor[1] == 3)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R5:
			tmp = avatar[4];
			if(cursor[1] == 4)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R6:
			tmp = avatar[5];
			if(cursor[1] == 5)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R7:
			tmp = avatar[6];
			if(cursor[1] == 6)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
		case DMC_G_R8:
			tmp = avatar[7];
			if(cursor[1] == 7)
			{
				tmp = cursor_on ? setCursor(tmp) : tmp;
			}
			transmit_data_column_G(createColumn(tmp));
			break;
	}
	return state;
}