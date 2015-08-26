#include <avr/io.h>
#include "timer.h"
#include <stdio.h>

#define ALLB N && P && E
#define NEXTB N && !P && !E
#define PREVB P && !E && !N
#define ENTERB E && !P && !N

#define CBS currBoard[i][j]

#define PLAYER1 1
#define PLAYER2 2
#define EMPTY 0
#define BLUE 1
#define RED 2
#define BLUEP 3
#define REDP 4
#define BOTHP 5
#define CURRSEL 6

#define ROWS 8
#define COLUMNS 8
#define POSSI 60

unsigned char difficulty = 0;
unsigned char mode = 0;
unsigned char lcdtext[32];

unsigned char currboard[ROWS][COLUMNS];

enum menu_SM {title, play2, play2Go, play1, play1Go, diffInc, diffDec, res, res_comfirm, reseted };
enum play2_states {init, findt,find2, wait_move, nextspot, prevspot, place, check_win, victory};

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

int menu_tick(){
	static menu_SM menuState;
	switch(menuState){
		case title:
			if(N || P || E){
				menuState = play2;
			}
			else{
				menuState = title;
			}
		break;

		case play2:
			if(ALLB){
				menuState = title;
			}
			else if (NEXTB){
				menuState = play1;
			}
			else if (PREVB){
				menuState = res;
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
				menuState = title;
			}
			else{
				menuState = play2Go;
			}
		break;

		case play1:
			if(ALLB){
				menuState = title;
			}
			else if (NEXTB){
				menuState = diffInc;
			}
			else if (PREVB){
				menuState = diffDec;
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
				menuState = title;
			}
			else{
				menuState = play1Go;
			}
		break;

        case diffInc:
            if(difficulty <= 2){
                menuState = play1;
            }
            else if(difficulty > 2){
                menuState = res;
            }
            else {
                menuState = title;
            }
            break;

		case diffDec:
			menuState = play1;
			break;

		case res:
			if(NEXTB){
				menuState = play2;
			}
			else if (ENTERB){
				menuState = res_comfirm;
			}
			else if (ALLB){
				menuState = title;
			}
			else if(PREVB){
				menuState = play1;
			}
			else {
				menuState = res;
			}
			break;

		case res_comfirm:
			if(NEXTB){
				menuState = reseted;
			}
			else if (PREVB){
				menuState = res;
			}
			else if (ALLB){
				menuState = title;
			}
			else{
				menuState = res_comfirm;
			}
			break;

		case reseted:
			if(N || B || P){
				menuState = title;
			}
			else{
				menuState = reseted;
			}
			break;
	}

	switch(menuState){
		case title:
			lcdtext = "    Othello!      High Score: ";
			mode = 0;
			difficulty = 0;
			break;

		case play2:
			lcdtext = "   2 players       High Score: ";
			difficulty = 0;
			break;
		case play2Go:
			mode = 2;
			difficulty = 0;
			break;
		case play1:
		lcdtext = "    1 player       High Score: ";
			break;
		case play1Go:
			mode = 1;
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
			break;
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


#define LEDSET bluTemp = bluTemp << j;\
                redTemp = redTemp << j;
int ledMatrix_SM{
    for(int i = 0; i < ROWS; ++i){
        unsigned char currRow = 0xFE;
        unsigned char redDis = 0x00;
        unsigned char bluDis = 0x00;
        transmit_dataC(currRow << i);
        for(int j = 0; j < COLUMNS; ++j){
            unsigned char bluTemp = 0x01;
            unsigned char redTemp = 0x01;
            if(CBS == BLUE){
                LEDSET
                bluDis |= bluTemp;
                redDis &= ~redTemp;
            }
            else if(CBS == RED){
                LEDSET
                bluDis &= ~bluTemp;
                redDis |= redTemp;
            }
            else if(CBS == BLUEP || CBS == REDP || CBS == BOTHP){
                LEDSET
                bluDis |= bluTemp;
                redDis |= redTemp;
            }
            else{
                bluTemp = ~(bluTemp << j);
                redTemp = ~(redTemp << j);
                bluDis &= bluTemp;
                redDis &= redTemp;
            }
        }
        transmit_dataB(bluDis);
        PORTD = redDisp;
    }
}

#define MODERES if(mode == 0){\
                    p_state = init;\
                }
int play_SM(){
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
}


main(void){

}
