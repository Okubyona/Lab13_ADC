/*	Author: Andrew Bazua [abazu001]
*  Partner(s) Name:
*	Lab Section:
*	Assignment: Lab #13  Exercise #1
*	Exercise Description: [optional - include for your own benefit]
*
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*
*  Demo Link:
*  https://drive.google.com/file/d/1332EUbfeBhiNS1egqdwfw08wj26ukHYP/view?usp=sharing
*
*/
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"

void ADC_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

typedef enum ADC_states { adc } adc_s;
typedef enum ledShiftTick_states {init_ledS, wait_ledS, left, right} ledShift;

int ADCTick(int state);
int ledShiftTick( int state);


int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;


    ADC_init();
    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = ADC;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ADCTick;

    task2.state = init_ledS;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ledShiftTick;

    unsigned long GCD = tasks[0]->period;
    for (unsigned char i = 0; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    while (1) {
        for (unsigned char i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }

    return 1;
}

// Shared variables
    // ADC value
    unsigned short x;

// ----------------------------

int ADCTick(int state) {
    state = adc;

    x = ADC;

    return state;
}
// Joystick values
// Rest: 504, Min: 15, Max: 1004

int ledShiftTick(int state) {
    static unsigned char row;
    static unsigned char column;

    switch (state) {
        case init_ledS: state = wait_ledS;
            row = 0xFE;
            column = 0x10;
            break;

        case wait_ledS:
            if (x >  400 && x < 600) {
                state = wait_ledS;
            }
            else if (x >= 600) { state = right; }
            else if (x <= 400) { state = left; }
            break;

        case left:
            if (x <= 400) { state = left; }
            else if (x >  400 && x < 600) {
                state = wait_ledS;
            }
            else if (x >= 600) { state = right; }

        case right:
            if (x >= 600) { state = right; }
            else if (x >  400 && x < 600) { state = wait_ledS; }
            else if (x <= 400) { state = left; }

    }

    switch (state) {
        case init_ledS:
            row = 0xFE;
            column = 0x10;
            break;

        case wait_ledS: break;

        case left:
            if(column == 0x80){
                column = 0x01;
            }
            else{ column <<= 1; }
            break;

        case right:
            if (column == 0x01) {
                column = 0x80;
            }
            else { column >>= 1; }
            break;
    }

    PORTC = column;
    PORTD = row;

    return state;
}
