#include <avr/io.h>
#include "timer.h"
#include <stdio.h>

#define ROWS 8
#define COLUMNS 8
#define POSSI 60
unsigned char currboard[ROWS][COLUMNS];

#define CBS currBoard[i][j]

#define N !(PINB & 0x40)
#define P !(PINB & 0x20)
#define ENT !(PINB & 0x80)
#define ALLB N && P && ENT
#define NEXTB N && !P && !ENT
#define PREVB P && !ENT && !N
#define ENTERB ENT && !P && !N


#define PLAYER1 1
#define PLAYER2 2
#define EMPTY 0
#define BLUE 1
#define RED 2
#define BLUEP 3
#define REDP 4
#define BOTHP 5
#define CURRSEL 6
#define changeturn turn = (turn+1)%2;

//EEPROM Macros
#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define update_eeprom_word(address,value) eeprom_update_word ((uint16_t*)address,(uint16_t)value)
unsigned char EEMEM eeprom_highscore;

unsigned char difficulty = 0;
unsigned char mode = 0;
unsigned char turn =0;
unsigned char lcdtext[32];

typedef struct task{
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
} task;

const unsigned long tasksPeriodGCD = 1;

enum play2_states {init, findt,find2, wait_move, nextspot, prevspot, place, check_win, victory};
enum menu_SM {initm, title, play2, play2Go, play1, play1Go,countchips, diffInc, diffDec, res, res_comfirm, reseted };
enum matrix_States {start, display, bstate};

void TimerISR(){
    unsigned char i;
    for(i = 0; i < taskNum; ++i){
        if(tasks[i].elapsedTime >= tasks[i].period){
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriodGCD;
    }
}
void initBoard(){
    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLUMNS; ++j){
            currboard[i][j] = 0;
        }
    }
	currboard[3][3] = 1;
	currboard[4][4] = 1;
	currboard[4][3] = 2;
	currboard[3][4] = 2;
}

//TODO: create more efficient function that uses recursion to update/check surrounding cells that were just updated.
int findSpots( int* spots[][POSSI], bool turn){
	unsigned char currPlayer = PLAYER1, enemy = PLAYER2;
	if(turn){
		currPlayer = PLAYER2;
		enemy = PLAYER1;
	}
	unsigned char oppoCnt = 0;
	int foundSpots = 0;
	for(int i = 0; i < ROWS; ++i){
		for(int j = 0; j < COLUMNS; ++j){
			if(currboard[i][j] == currPlayer){
				oppoCnt = 0;
				for(int x = i+1; x < ROWS; ++x){
					if(currboard[x][j] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = j;
						foundSpots++;
						break;
					}
					else if(currboard[x][j] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int x = i-1; x < ROWS && x >= 0; --x){
					if(currboard[x][j] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = j;
						foundSpots++;
						break;
					}
					else if(currboard[x][j] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j+1; y < COLUMNS; ++y){
					if(currboard[i][y] == 0  && oppoCnt){
						spots[0][foundSpots] = i;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[i][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j-1; y < COLUMNS && y >= 0; --y){
					if(currboard[i][y] == 0  && oppoCnt){
						spots[0][foundSpots] = i;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[i][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j+1, x = i+1; y < COLUMNS && y >= 0 && x < ROWS && x >= 0;++y, ++x){
					if(currboard[x][y] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[x][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j+1, x = i-1; y < COLUMNS && y >= 0 && x < ROWS && x >= 0;++y, --x){
					if(currboard[x][y] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[x][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j-1, x = i+1; y < COLUMNS && y >= 0 && x < ROWS && x >= 0;--y, ++x){
					if(currboard[x][y] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[x][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
				oppoCnt = 0;
				for(int y = j-1, x = i-1; y < COLUMNS && y >= 0 && x < ROWS && x >= 0;--y, --x){
					if(currboard[x][y] == 0  && oppoCnt){
						spots[0][foundSpots] = x;
						spots[1][foundSpots] = y;
						foundSpots++;
						break;
					}
					else if(currboard[x][y] == enemy){
						oppoCnt++;
					}
					else{
						break;
					}
				}
			}
		}
	}
	return foundSpots;
}

void clr_possi(unsigned char* currboard[][COLUMNS], int spots[][POSSI], int possible_spots){
	for(int i = 0; i < possible_spots; ++i){
		if(!(currboard[spots[1][i]][spots[2][i]] == 1 || currboard[spots[1][i]][spots[2][i]] == 2)){
			currboard[spots[1][i]][spots[2][i]] = 0;
		}
	}
}

unsigned char chipNum(int color){
    int chipCount = 0;
    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLUMNS; ++j){
            if(currboard[i][j] == color){
                 chipCount++;
            }
        }
    }
    return chipCount;
}

#define DISPLAYHS   LCD_Cursor(30);\
                    LCD_WriteData(hs/10+'0');\
                    LCD_Cursor(31);\
                    LCD_WriteData(hs/10+'0');
int menu_tick(int menuState){
    static unsigned char hs;
    static unsigned char prevturn;
    unsigned char countedchips;
	switch(menuState){
        case initm:
            if(!N && !P && !ENT){
                menuState = title;
                LCD_DisplayString(1, "    Othello!     Press Any Key");
            }
            else{
                menuState = initm;
            }
		case title:
			if(N || P || ENT){
				menuState = play2;
                LCD_DisplayString(1, "   2 players     High Score: ");
                DISPLAYHS
			}
			else{
				menuState = title;
			}
		break;

		case play2:
			if(ALLB){
				menuState = initm;
			}
			else if (NEXTB){
				menuState = play1;
                LCD_DisplayString(1, " 1 player Easy   High Score: ");
                DISPLAYHS
			}
			else if (PREVB){
				menuState = res;
                LCD_DisplayString(1, "Reset High Score High Score: ");
                DISPLAYHS
			}
			else if (ENTERB){
				menuState = play2Go;
			}
			else{
				menuState = play2;
			}
		break;

		case play2Go:
			if(ALLB){
				menuState = initm;
			}
            else if(prevturn != turn){
                menuState = countchips;
            }
			else{
				menuState = play2Go;
			}
		break;

		case play1:
			if(ALLB){
				menuState = initm;
			}
			else if (NEXTB){
				menuState = diffInc;
			}
			else if (PREVB && difficulty >0){
				menuState = diffDec;
			}
            else if (PREVB && difficulty == 0){
				menuState = play2;
                LCD_DisplayString(1, "   2 players     High Score: ");
                DISPLAYHS
            }
			else if (ENTERB){
				menuState = play1Go;
			}
			else{
				menuState = play1;
			}
		break;

		case play1Go:
			if(ALLB){
				menuState = initm;
			}
            else if(prevturn != turn){
                menuState = countchips;
            }
			else{
				menuState = play1Go;
			}
		break;

        case countchips:
            if(ALLB){
                menuState = initm;
            }
            else if(mode == 1){
                menuState = play1Go;
            }
            else if(mode == 2){
                menuState = play2Go;
            }
            else{
                menuState = initm;
            }
            break;
        case diffInc:
            if(difficulty <= 2){
                menuState = play1;
                if(difficulty == 2){
                    LCD_DisplayString(1, " 1 player Hard   High Score: ");
                    DISPLAYHS
                }
                else if(difficulty == 1){
                    LCD_DisplayString(1, " 1 player Med.   High Score: ");
                    DISPLAYHS
                }
            }
            else if(difficulty > 2){
                menuState = res;
                LCD_DisplayString(1, "Reset High Score High Score: ");
                DISPLAYHS
            }
            else {
                menuState = initm;
            }
            break;

		case diffDec:
			menuState = play1;
            if(difficulty == 0){
                LCD_DisplayString(1, " 1 player Easy   High Score: ");
                DISPLAYHS
            }
            else if(difficulty == 1){
                LCD_DisplayString(1, " 1 player Med.   High Score: ");
                DISPLAYHS
            }
			break;

		case res:
			if(NEXTB){
				menuState = play2;
                LCD_DisplayString(1, "   2 players     High Score: ");
                DISPLAYHS
			}
			else if (ENTERB){
				menuState = res_comfirm;
                LCD_DisplayString(1, " Confirm Reset? Yes=Next No=Prev");
			}
			else if (ALLB){
				menuState = initm;
			}
			else if(PREVB){
				menuState = play1;
                LCD_DisplayString(1, " 1 player Hard   High Score: ");
                DISPLAYHS
			}
			else {
				menuState = res;
			}
			break;

		case res_comfirm:
			if(NEXTB){
				menuState = reseted;
                LCD_ClearScreen;
                LCD_DisplayString(1,"High Score Reset Press Any Key");
			}
			else if (PREVB){
				menuState = res;
                LCD_DisplayString(1, "Reset High Score High Score: ");
                DISPLAYHS
			}
			else if (ALLB){
				menuState = initm;
			}
			else{
				menuState = res_comfirm;
			}
			break;

		case reseted:
			if(N || ENT || P){
				menuState = initm;
			}
			else{
				menuState = reseted;
			}
			break;
	}

	switch(menuState){
        case initm:
            hs = read_eeprom_word(&eeprom_highscore);
		case title:
			mode = 0;
			difficulty = 0;
			break;

		case play2:
			difficulty = 0;
			break;
		case play2Go:
			mode = 2;
			difficulty = 0;
			break;
		case play1:
			break;
		case play1Go:
			mode = 1;
			break;
        case countchips:
            if(turn == 0){
                LCD_DisplayString(1,"   Turn: Blue   Blue:    Red:");
            }
            else{
                LCD_DisplayString(1,"   Turn: Red    Blue:    Red:");
            }
            countedchips = chipNum(BLUE);
            LCD_Cursor(22);
            LCD_WriteData(countedchips/10+'0');
            LCD_Cursor(23);
            LCD_WriteData(countedchips%10+'0');
            countedchips = chipNum(RED);
            LCD_Cursor(30);
            LCD_WriteData(countedchips/10+'0');
            LCD_WriteData(countedchips%10+'0');
            break;
		case diffInc:
			difficulty++;
			break;
		case diffDec:
			difficulty--;
			break;
		case res:
			break;
		case res_comfirm:
			break;
		case reseted:
			//reset eeprom;
            update_eeprom_word(&eeprom_highscore, 0);
			break;
	}
    prevturn = turn;
    return menuState;
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


#define BLUSET bluTemp = bluTemp << j;
#define REDSET redTemp = redTemp << j;
int ledMatrix_SM(int ledDis){
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
    return ledDis;
}

#define MODERES if(mode == 0){\
                    p_state = init;\
                }
int play_SM(int p_state){
    static play2_states p_state;
    static unsigned char max_cnt = 0;
    switch(p_state){
        case init:
			MODERES
            else if(mode == 2){
                p_state = findt;
            }
            else{
                p_state = init;
            }
            break;
        case findt:
            MODERES
            else{
				p_state = find2;
			}
            break;
        case find2:
			MODERES
            else if(max_cnt != 0){
                p_state = wait_move;
            }
            else{
                p_state = check_win;
            }
            break;
        case wait_move:
            MODERES
            else if(ENTERB){
                p_state = place;
            }
            else if(NEXTB){
                p_state = nextspot;
            }
            else if(PREVB){
                p_state = prevspot;
            }
            else{
                p_state = wait_move;
            }
            break;
		case nextspot:
			MODERES
            else{
				p_state = wait_move;
			}
			break;
		case prevspot:
			MODERES
			else{
				p_state = wait_move
			}
			break;
		case place:
			p_state = findt;
			break;
		case check_win:
			MODERES
			else if(end_count < 2)
				p_state = findt;
			}
			else{
				p_state = victory;
			}
		case victory:
			MODERES
			else{
				p_state = victory;
			}
		}
	switch(p_state){
		case init:
			initBoard();
		case findt:
			max_cnt = findspots();
	}
    return p_state;
}

task tasks[3];
const unsigned short taskNum = 3;
#define TASKINIT(taski,initstate, periodi, tf) tasks[taski].state = initstate;\
                       tasks[taski].period = periodi;\
                       tasks[taski].elapsedTime = tasks[taski].period; \
                       tasks[taski].TickFct = & tf;\
                       taski ++;
main(void){
    unsigned char currSM = 0;
    TASKINIT(currSM,initm,100,menu_tick)
    TASKINIT(currSM,init, 100,play_SM)
    TASKINIT(currSM,start,1,ledMatrix_SM)

    TimerSet(tasksPeriodGCD);
    TimerOn();

    while(1){
        Sleep();
    }
    return 0;
}
