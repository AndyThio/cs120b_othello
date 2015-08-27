/*
 * ledmatrixtest.c
 *
 * Created: 8/26/2015 2:12:46 PM
 *  Author: student
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#define ROWS 8
#define COLUMNS 8

unsigned char currboard[ROWS][COLUMNS];
#define CBS currboard[i][j]

#define PLAYER1 1
#define PLAYER2 2
#define EMPTY 0
#define BLUE 1
#define RED 2
#define BLUEP 3
#define REDP 4
#define BOTHP 5
#define CURRSEL 6




volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
    // AVR timer/counter controller register TCCR1
    TCCR1B  = 0x0B; // bit3 = 1: CTC mode (clear timer on compare)
    // bit2bit1bit0=011: prescaler /64
    // 00001011: 0x0B
    // SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
    // Thus, TCNT1 register will count at 125,000 ticks/s

    // AVR output compare register OCR1A.
    OCR1A   = 125;  // Timer interrupt will be generated when TCNT1==OCR1A
    // We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
    // So when TCNT1 register equals 125,
    // 1 ms has passed. Thus, we compare to 125.
    // AVR timer interrupt mask register

    TIMSK1  = 0x02; // bit1: OCIE1A -- enables compare match interrupt

    //Initialize avr counter
    TCNT1 = 0;

    // TimerISR will be called every _avr_timer_cntcurr milliseconds
    _avr_timer_cntcurr = _avr_timer_M;

    //Enable global interrupts
    SREG |= 0x80;   // 0x80: 1000000
}

void TimerOff() {
    TCCR1B  = 0x00; // bit3bit2bit1bit0=0000: timer off
}

void TimerISR() {
    TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
    // CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
    _avr_timer_cntcurr--;           // Count down to 0 rather than up to TOP
    if (_avr_timer_cntcurr == 0) {  // results in a more efficient compare
        TimerISR();                 // Call the ISR that the user uses
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
void transmit_dataB(unsigned char data){
    int i;
    for(i= 0; i< 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTB = 0x08;
        // set SER = next bit of data to be sent.
        PORTB|= ((data>> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTB|= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTB|= 0x04;
    // clears all lines in preparation of a new transmission
    PORTB= 0x00;
}

void transmit_dataC(unsigned char data){
    int i;
    for(i= 0; i< 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTC = 0x08;
        // set SER = next bit of data to be sent.
        PORTC|= ((data>> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTC|= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTC|= 0x04;
    // clears all lines in preparation of a new transmission
    PORTC= 0x00;
}

enum matrix_States {start, display, bstate} ledDis;

#define BLUSET bluTemp = bluTemp << j;
#define REDSET redTemp = redTemp << j;
int ledMatrix_SM(){
    unsigned char currRow;
    unsigned char redDis;
    unsigned char bluDis;
    unsigned char bluTemp;
    unsigned char redTemp;
    static int i;

    switch(ledDis){
        case start:
            ledDis = display;
            break;
        case display:
            ledDis = bstate;
            break;
        case bstate:
            ledDis = display;
            break;
        default:
            ledDis = start;
            break;
    }
    switch(ledDis){
        case start:
            i = 0;
            break;
        case display:
            currRow = 0x01;
            redDis = 0x00;
            bluDis = 0x00;
            transmit_dataC(currRow << i);
            for(int j = 0; j < COLUMNS; ++j){
                redTemp = 0x01;
                if(CBS == BLUE){
                    REDSET
                    redDis &= ~redTemp;
                }
                else if(CBS == RED){
                    REDSET
                    redDis |= redTemp;
                }
                else if(CBS == BLUEP || CBS == REDP || CBS == BOTHP){
                    REDSET
                    redDis |= redTemp;
                }
                else{
                    redTemp = ~(redTemp << j);
                    redDis &= redTemp;
                }
            }
            transmit_dataB(~bluDis);
            PORTD = ~redDis;
            i++;
            if(i >= ROWS){
                i = 0;
            }
            break;

        case bstate:
            currRow = 0x01;
            redDis = 0x00;
            bluDis = 0x00;
            transmit_dataC(currRow << i);
            for(int j = 0; j < COLUMNS; ++j){
                bluTemp = 0x01;
                if(CBS == BLUE){
                    BLUSET
                    bluDis |= bluTemp;
                }
                else if(CBS == RED){
                    BLUSET
                    bluDis &= ~bluTemp;
                }
                else if(CBS == BLUEP || CBS == REDP || CBS == BOTHP){
                    BLUSET
                    bluDis |= bluTemp;
                }
                else{
                    bluTemp = ~(bluTemp << j);
                    bluDis &= bluTemp;
                }
            }
            transmit_dataB(~bluDis);
            PORTD = ~redDis;
            break;
    }
    return 0;
}


int main(void)
{
    DDRB = 0x0F; PORTB = 0xF0;
    DDRC = 0x0F; PORTC = 0xF0;
    DDRD = 0xFF; PORTD = 0x00;
    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLUMNS; ++j){
            currboard[i][j] = BLUE;
        }
    }
    TimerSet(5);
    TimerOn();
    while(1)
    {
        ledMatrix_SM();
        while(!TimerFlag);
        TimerFlag = 0;
    }
}
