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
*  https://drive.google.com/file/d/1AviS_HPDK7vzmYncV5J-sAwbPeC68cZW/view?usp=sharing
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
typedef enum ledMoveSpeed_states { getSpeed} speed;


int ADCTick(int state);
int ledShiftTick( int state);
int ledMoveSpeedTick( int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;


    ADC_init();
    static task task1, task2, task3;
    task *tasks[] = {&task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = ADC;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ADCTick;

    task2.state = init_ledS;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ledShiftTick;

    task3.state = getSpeed;
    task3.period = 10;
    task3.elapsedTime = task3.period;
    task3.TickFct = &ledMoveSpeedTick;

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
    unsigned char speedVal;

// ----------------------------

int ADCTick(int state) {
    state = adc;

    x = ADC;

    return state;
}

int ledShiftTick(int state) {
    static unsigned char row;
    static unsigned char column;
    static unsigned char count;

    switch (state) {
        case init_ledS: state = wait_ledS;
            row = 0xFE;
            column = 0x10;
            break;

        case wait_ledS:
            if (x >  500 && x < 600) {
                state = wait_ledS;
            }
            else if (x >= 600) { state = right; }
            else if (x <= 500) { state = left; }
            break;

        case left:
            if (x <= 500) { state = left; }
            else if (x >  500 && x < 600) {
                state = wait_ledS;
            }
            else if (x >= 600) { state = right; count = 0; }
            break;

        case right:
            if (x >= 600) { state = right; }
            else if (x >  500 && x < 600) { state = wait_ledS; }
            else if (x <= 500) { state = left;  count = 0; }
            break;

    }

    switch (state) {
        case init_ledS:
            row = 0xFE;
            column = 0x10;
            break;

        case wait_ledS:
            count = 0;
            break;

        case left:
            if (count == speedVal) {
                if(column == 0x80){
                    column = 0x01;
                }
                else{ column <<= 1; }
                count = 0;
            }
            ++count;
            break;

        case right:
            if (count == speedVal) {
                if (column == 0x01) {
                    column = 0x80;
                }
                else { column >>= 1; }
                count = 0;
            }
            ++count;
            break;
    }

    PORTC = column;
    PORTD = row;

    return state;
}

// Joystick values
// Rest: 504, Min: 15, Max: 1004

int ledMoveSpeedTick(int state) {
    state = getSpeed;

    if (((x >= 600) && (x < 700)) || ((x <= 500) && (x > 400))) { speedVal = 100; }

    else if (((x >= 700) && (x < 800)) || ((x <= 400) && (x < 350))) { speedVal = 50; }

    else if (((x >= 800) && (x < 900)) || ((x <= 350) && (x < 250))) { speedVal = 25; }

    else if ((x >= 900) || (x <= 250)) { speedVal = 10; }



    return state;
}
