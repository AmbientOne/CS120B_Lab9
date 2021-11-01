/*	Author: Abdirahman Abdi
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #9  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo link: 
 */
#include <avr/io.h>
#include "io.h"
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif






volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned char currentNote = 0;
unsigned char timeTrigger = 0;
unsigned long _avr_timer_cntcurr = 0;
void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
 TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
	    if (!frequency) { TCCR3B &= 0x08; }
     	    else { TCCR3B |= 0x03; }
     	    if (frequency < 0.954) { OCR3A = 0xFFFF; }
     	    else if (frequency > 31250) { OCR3A = 0x0000; }
     	    else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }
     	    TCNT3 = 0;
     	    current_frequency = frequency;
   }
 }

 void PWM_on() {
	TCCR3A = (1 << COM3A0);
   	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
   	set_PWM(0);
 }

 void PWM_off() {
   	TCCR3A = 0x00;
   	TCCR3B = 0x00;
 }

enum States {Start, Init, On, changeNote, checkIfPressed} state;
unsigned char j = 0;
unsigned char timerVal = 0;
double notes[15] = {392.00, 440.00, 493.88, 261.63, 293.66, 329.63, 261.63, 440.00, 523.25, 293.66, 440.00, 493.88, 261.63, 349.23, 523.25};
unsigned char time[15] = {2,3,3,4,6,5,3,2,4,8,2,2,2,2,2};
unsigned char ticks = 0;

void Tick(){
    unsigned char press = ~PINA & 0x07;
	switch(state){
	    case Start:
		    state = Init;
		    break;
	    case Init:
		    if(press == 0x01)state = On;
            else state = Init;
		    break;
	    case On:
		    if (ticks <= time[j]){
                state = On;
         	}else if(ticks > time[j]){
                state = changeNote;
         	}
         	break;
	    case changeNote:
		    if(timerVal<15){
                state = On;
		    }else if(timerVal == 15){
                state = checkIfPressed;
			    set_PWM(0);
		    }
		    break;
	    case checkIfPressed:
		    if(press == 0x01) state = checkIfPressed;
		    else state = Init;
		    break;
	    default:
		    state = Start;
		    break;


	    switch (state) {
		    case Start:
			    break;
		    case Init:
                currentNote=0; 
                j=0;
                timerVal=0;
                ticks = 0;
                set_PWM(0);
			    break;
		    case On: 
                if (ticks <= time[j]){
                set_PWM(notes[currentNote]); 
                ticks++;
         	    }else if(ticks > time[j]){
                timerVal++;
         	}	
				break;
		    case changeNote:
                if(timerVal<15){
                    ticks = 0;
                    currentNote++;
                    j++;
                }else if(timerVal == 15){
                    set_PWM(0);
                }
				break;
		    case checkIfPressed:
				break;
		    default:	
	  		    break;
	    }
	}
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    state = Start;
    TimerSet(100);
    TimerOn();
	PWM_on();
    while (1) {
        Tick();
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}