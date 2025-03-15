#include <stm32f031x6.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "sound.h"
#include "serial.h"
#define MAX_SIZE 30
#define MIN_Y 20
#define MAX_Y 131
#define MIN_X 10
#define MAX_X 110


void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);

volatile uint32_t milliseconds;

// images
const uint16_t snake[]=
{
	40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,
};

const uint16_t snakehead2H[]=
{
	40725,40725,40725,40725,65535,65535,0,0,40725,40725,40725,40725,65535,65535,0,0,40725,40725,40725,40725,65535,65535,65535,65535,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,65535,65535,65535,65535,40725,40725,40725,40725,65535,65535,0,0,40725,40725,40725,40725,65535,65535,0,0,
};

const uint16_t snakehead2V[]=
{
0,0,65535,40725,40725,65535,0,0,0,0,65535,40725,40725,65535,0,0,65535,65535,65535,40725,40725,65535,65535,65535,65535,65535,65535,40725,40725,65535,65535,65535,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,40725,
};

const uint16_t snakeheadH[]=
{
	22,22,22,22,65535,65535,0,0,22,22,22,22,65535,65535,0,0,22,22,22,22,65535,65535,65535,65535,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,65535,65535,65535,65535,22,22,22,22,65535,65535,0,0,22,22,22,22,65535,65535,0,0,
};

const uint16_t snakeheadV[]=
{
	0,0,65535,22,22,65535,0,0,0,0,65535,22,22,65535,0,0,65535,65535,65535,22,22,65535,65535,65535,65535,65535,65535,22,22,65535,65535,65535,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,
};

const uint16_t apple[]=
{
0,0,0,0,0,43008,0,0,0,41236,24579,0,43008,0,0,0,0,0,49973,40979,3410,39680,0,0,0,23841,65329,7985,55849,56922,48152,0,0,7482,39985,15409,23601,40754,64561,0,0,7482,39985,15409,15409,64561,15409,0,0,23841,16170,64561,23601,15409,7193,0,0,0,23874,64561,31017,7193,0,0,
};


void move(int direction);
void move2(int direction);

void updateBody1();
void updateBody2();

void checkforCollision1(int);
void checkforCollision2(int);

void mainMenu();
void creditScroll();

int x = 60;
int y = 70;
int x2 = 70;
int y2 = 70;
int snakeBody1[3][MAX_SIZE] = {0};
int snakeBody2[3][MAX_SIZE] = {0};

short gameOverVar = 0;
short currentCycle = 1;

short appleX = 0;
short appleY = 0;
uint8_t appleEaten = 0;
uint8_t is2Player = 0;

uint16_t musicSequence[] = { 50, 200, 50, 150, 50, 200, 50, 30 };

void drawBorder();
void sneyk1P();
void sneyk2P();
void spawnApple();
void gameOver(short);
void gameOver2(short);
void orangeOn(void);
void orangeOff(void);
void greenOn(void);
void greenOff(void);

void printBanner();

int main()
{
	initClock();
	initSysTick();
	setupIO();
	initSound();
	pinMode(GPIOA,1,1); // Make GPIOA bit 1 an output
	pinMode(GPIOA,0,1); // Make GPIOA bit 0 an output

	initSerial();

	printBanner();
	
	mainMenu();
	return 0;
}

void printBanner()
{
	eputs("\n");
	eputs(" ,---.  ,--.  ,--.,------.,--.   ,--.,--. ,--. ");
	eputs("\n");
	eputs("'   .-' |  ,'.|  ||  .---' \\  `.'  / |  .'   / ");
	eputs("\n");
	eputs("`.  `-. |  |' '  ||  `--,   '.    /  |  .   '  ");
	eputs("\n");
	eputs(".-'    ||  | `   ||  `---.    |  |   |  |\\   \\ ");
	eputs("\n");
	eputs("`-----' `--'  `--'`------'    `--'   `--' '--' ");
	eputs("\n");
	
}

void gameOver(short end1)
{
    currentCycle = 1;
    gameOverVar = 1;

	//this chunk of code flashes the snake to highlight that it has died
    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, 255);
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, RGBToWord(0xff, 0xff, 0));
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, 255);
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, RGBToWord(0xff, 0xff, 0));
    }

    fillRectangle(0, 0, 128, 186, 0);
	
	

	if(is2Player)
	{
		// green died, turn on orange LED to show orange (P2) has won
		orangeOn();
		greenOff();
		printTextX2("P2 WINS", 25, 90, RGBToWord(255, 255, 255), 0);
	}

    for (int i = 100; i >= 0; i-=10)
    {
        fillRectangle(0, 40, 128, 30, 0);
        printTextX2(" GAME OVER ", i, 50, RGBToWord(255, 255, 255), 0);
        delay(200);
    }

	if(is2Player)
	{
		//triumphant note -- someone won
		playNote(500);
		delay(200);
		playNote(700);
		delay(200);
		playNote(750);
		delay(400);
		playNote(0);
		delay(1200);
	}
	else
	{
		//sad note -- only player lost
		playNote(220);
		delay(800);
		playNote(150);
		delay(1200);
		playNote(0);
	}

}

void gameOver2(short end1)
{
    currentCycle = 1;
    gameOverVar = 1;

	//this chunk of code flashes the snake to highlight that it has died
    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, 255);
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, RGBToWord(0xff, 0xff, 0));
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, 255);
    }

    delay(500);

    for (int j = 0; j <= end1; j++)
    {
        fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, RGBToWord(0xff, 0xff, 0));
    }
	
	// orange died, turn on green LED to show green (P1) has won
	orangeOff();
	greenOn();

    fillRectangle(0, 0, 128, 186, 0);

	if(is2Player)
	{
		printTextX2("P1 WINS", 25, 90, RGBToWord(255, 255, 255), 0);
	}

    for (int i = 100; i >= 0; i-=10)
    {
        fillRectangle(0, 40, 128, 30, 0);
        printTextX2(" GAME OVER ", i, 50, RGBToWord(255, 255, 255), 0);
        delay(200);
    }

    if(is2Player)
	{
		//triumphant note -- someone won
		playNote(500);
		delay(200);
		playNote(700);
		delay(200);
		playNote(750);
		delay(400);
		playNote(0);
		delay(1200);
	}
	else
	{
		//sad note -- only player lost
		playNote(220);
		delay(800);
		playNote(150);
		delay(1200);
		playNote(0);
	}

}

void orangeOn()
{
    GPIOA->ODR |= (1 << 1);
}
void orangeOff()
{
    GPIOA->ODR &= ~(1 << 1);
}

void greenOn()
{
    GPIOA->ODR |= (1 << 0);
}
void greenOff()
{
    GPIOA->ODR &= ~(1 << 0);
}

void sneyk1P()
{
	greenOff();
	orangeOff();
	short p1Direction = 0; // 0 - R, 1 - D, 2 - L, 3 - U
	currentCycle = 1;

	drawBorder();

	snakeBody1[0][0] = 1; //fill head, 2 segments behind
	snakeBody1[0][1] = 1;
	snakeBody1[0][2] = 1;

	spawnApple();

	while(gameOverVar == 0)
	{
		
		//getting input from board
		if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
		{			
			if (p1Direction != 2)
			{
				p1Direction = 0;	
			}	
		}
		if ((GPIOB->IDR & (1 << 5)) == 0) // left pressed
		{			
			if (p1Direction != 0)
			{
				p1Direction = 2;	
			}			
		}
		if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
		{
			if (p1Direction != 3)
			{
				p1Direction = 1;	
			}	
		}
		if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{
			if (p1Direction != 1)
			{
				p1Direction = 3;	
			}	
		}
		
		//sneyk1P runs every 50ms, this makes snake move only every 400ms
		if(currentCycle == 8)
		{
			move(p1Direction);
			if(gameOverVar == 0)
			{
				//plays snake moving noise
				playNote(50);
			}			
			
			currentCycle = 1;
		}
		else if (currentCycle == 1)
		{
			//stops playing noise
			playNote(0);
			currentCycle++;
		}
		else
		{
			currentCycle++;
		}

		//play sound for 2 cycles when eat apple
		if (appleEaten == 3)
		{
			playNote(0);
			appleEaten = 0;
		}
		else if (appleEaten == 2)
		{
			appleEaten++;
		}
		else if (appleEaten == 1)
		{
			playNote(800);
			appleEaten = 2;
		}

		delay(50);
	}
}

void sneyk2P()
{
	short p1Direction = 2;
	short p2Direction = 0; // 0 - R, 1 - D, 2 - L, 3 - U
	currentCycle = 1;
	char p2Input = '0';
	short endofSnake1 = 2;
	short endofSnake2 = 2;

	is2Player = 1;

	drawBorder();

	snakeBody1[0][0] = 1; //fill head, 2 segments behind
	snakeBody1[0][1] = 1;
	snakeBody1[0][2] = 1;

	snakeBody2[0][0] = 1; //fill head, 2 segments behind
	snakeBody2[0][1] = 1;
	snakeBody2[0][2] = 1;

	spawnApple();

	while(gameOverVar == 0)
	{
	
		//getting input from board
		if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
		{	
			//if prevents snake from going 180* and eating itself immediately		
			if (p1Direction != 2)
			{
				p1Direction = 0;	
			}	
		}
		else if ((GPIOB->IDR & (1 << 5)) == 0) // left pressed
		{			
			if (p1Direction != 0)
			{
				p1Direction = 2;	
			}			
		}
		else if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
		{
			if (p1Direction != 3)
			{
				p1Direction = 1;	
			}	
		}
		else if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{
			if (p1Direction != 1)
			{
				p1Direction = 3;	
			}	
		}
		
		//get p2 input from serial
		p2Input = egetchar();

		//serial input --> snake direction
		switch(p2Input)
		{
			case 'w':
			{
				//if prevents snake from going 180* and eating itself immediately		
				if (p2Direction != 1)
				{
					p2Direction = 3;	
				}
				break;
			}

			case 'd':
			{
				if (p2Direction != 2)
				{
					p2Direction = 0;	
				}
				break;
			}

			case 's':
			{
				if (p2Direction != 3)
				{
					p2Direction = 1;	
				}
				break;
			}

			case 'a':
			{
				if (p2Direction != 0)
				{
					p2Direction = 2;	
				}
				break;
			}

			default:
			{
				break;
			}

		}

		//sneyk2P runs every 50ms, this makes snakes move only every 400ms
		if(currentCycle == 8)
		{	
			move2(p2Direction);		
			move(p1Direction);
			if(gameOverVar == 0)
			{
				//plays snake moving noise
				playNote(50);
			}	

			currentCycle = 1;
		}
		else if (currentCycle == 1)
		{
			currentCycle++;
			
			//stops playing noise
			playNote(0);
		}
		else
		{
			currentCycle++;
		}

		//play sound for 2 cycles when eat apple
		if (appleEaten == 3)
		{
			playNote(0);
			appleEaten = 0;
		}
		else if (appleEaten == 2)
		{
			appleEaten++;
		}
		else if (appleEaten == 1)
		{
			playNote(800);
			appleEaten = 2;
		}

		// comparing lenghts of snake to activate LEDs to show which one is in the lead
		while( snakeBody1[0][endofSnake1] != 0)
		{
			endofSnake1++;
		}
		endofSnake1--;

		while( snakeBody2[0][endofSnake2] != 0)
		{
			endofSnake2++;
		}
		endofSnake2--;

		//snake1 is green, snake2 is orange
		if(endofSnake1 > endofSnake2)
		{
			orangeOff();
			greenOn();
		}
		else if(endofSnake2 > endofSnake1)
		{
			greenOff();
			orangeOn();
		}
		else
		{
			// if they are equal, neither light is on
			orangeOff();
			greenOff();
		}
		
		delay(50);
	}
}

void creditScroll()
{
	
	fillRectangle(0,0,128,154,0);

	// prints over and over at different y to 'scroll'
	for (int i = 140; i >= 1; i--)
	{
		printTextX2("DORIAN", 10, i, RGBToWord(0, 50, 255), 0);
		printTextX2("          ", 10, i + 14, 0, 0);
		printTextX2("DZIGUMOVIC", 10, i + 28, RGBToWord(0, 50, 255), 0);
		printTextX2("          ", 10, i + 42, 0, 0);
		printTextX2("KENTON", 10, i + 56, RGBToWord(0, 255, 0), 0);
		printTextX2("          ", 10, i + 70, 0, 0);
		printTextX2("KAMTCHOU", 10, i + 84, RGBToWord(0, 255, 0), 0);
		printTextX2("          ", 10, i + 98, 0, 0);
		if (i < 48)
		{
			printTextX2("NATHAN", 10, i + 112, RGBToWord(255, 0, 0), 0);
			printTextX2("          ", 10, i + 126, 0, 0);
			printTextX2("DANIEL", 10, i + 140, RGBToWord(255, 0, 0), 0);
			printTextX2("          ", 10, i + 154, 0, 0);

		}
		delay(15);
	}

	delay(3000);
	fillRectangle(0,0,128,156,0);
	printTextX2("__________", 5, 8, RGBToWord(0xff,0xff,0), 0);
	printTextX2("SNEYK", 35, 0, RGBToWord(0xff,0xff,0), 0);
}

void mainMenu()
{
	int currentSelection = 0;
	uint32_t currentTime = milliseconds;
	uint32_t musicTime = milliseconds;
	uint8_t musicToggle = 0;
	int currentNote = 0;
	fillRectangle(0,0,128,154,0);
	printTextX2("__________", 5, 8, RGBToWord(0xff,0xff,0), 0);
	printTextX2("SNEYK", 35, 0, RGBToWord(0xff,0xff,0), 0);
	

	while(1)
	{
		if(gameOverVar == 1)
        {
            x = 60;
            y = 70;
			x2 = 70;
			y2 = 70;
			is2Player = 0;
            fillRectangle(0,0,128,154,0);
            printTextX2("__________", 5, 8, RGBToWord(0xff,0xff,0), 0);
            printTextX2("SNEYK", 35, 0, RGBToWord(0xff,0xff,0), 0);
            gameOverVar = 0;
			for(int k = 0; k < MAX_SIZE; k++)
            {
				//fill arrays with 0
                snakeBody1[0][k] = 0;
				snakeBody1[1][k] = 0;
				snakeBody1[2][k] = 0;

				snakeBody2[0][k] = 0;
				snakeBody2[1][k] = 0;
				snakeBody2[2][k] = 0;
            }
        }
		if(milliseconds - currentTime > 150)
		{
			if ( (GPIOA->IDR & (1 << 11)) == 0) // down pressed
			{
				if(currentSelection == 2)
				{
					currentSelection = 0;
				}
				else
				{
					currentSelection++;
				}
				currentTime = milliseconds;
			}
			else if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
			{
				if(currentSelection == 0)
				{
					currentSelection = 2;
				}
				else
				{
					currentSelection--;
				}
				currentTime = milliseconds;
			}
		}
		
		if(milliseconds - musicTime > 200)
		{
			playNote(0);
			musicTime = milliseconds;
			musicToggle = 0;
		}
		else if(milliseconds - musicTime > 130)
		{
			if (musicToggle == 0)
			{
				playNote(musicSequence[currentNote]);
				if(currentNote == 7)
				{
					currentNote = 0;
				}
				else
				{
					currentNote++;
				}
				musicToggle = 1;
			}
			
		}


		switch(currentSelection)
		{
			case 0:
			{
				printTextX2("1 PLAYER >", 10, 40, 255, 0);
				printTextX2("2 PLAYER  ", 10, 70, RGBToWord(0xff,0xff,0), 0);
				printTextX2("CREDITS  ", 10, 100, RGBToWord(0xff,0xff,0), 0);

				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{
					playNote(700);
					delay(80);
					playNote(0);			
					sneyk1P();	
				}


				break;
			}

			case 1:
			{
				printTextX2("1 PLAYER  ", 10, 40, RGBToWord(0xff,0xff,0), 0);
				printTextX2("2 PLAYER >", 10, 70, 255, 0);
				printTextX2("CREDITS  ", 10, 100, RGBToWord(0xff,0xff,0), 0);
				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{	
					playNote(700);
					delay(80);
					playNote(0);				
					sneyk2P();	
				}

				break;
			}

			case 2:
			{
				printTextX2("1 PLAYER  ", 10, 40, RGBToWord(0xff,0xff,0), 0);
				printTextX2("2 PLAYER  ", 10, 70, RGBToWord(0xff,0xff,0), 0);
				printTextX2("CREDITS >", 10, 100, 255, 0);
				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{	
					playNote(700);
					delay(80);
					playNote(0);	
					creditScroll();	
				}
				
				break;
			}
		}
		
	}

}

void move(int direction)
{
	

	switch(direction)
	{
		case 0: // right
		{
			if (x < MAX_X)
			{
				x = x + 10;
				// draw head
				putImage(x,y,8,8,snakeheadH,0,0);
			}
			break;
		}

		case 1: // down
		{
			if (y < MAX_Y)
			{
				y = y + 10;
				// draw head
				putImage(x,y,8,8,snakeheadV,0,1);
			}
			break;
		}

		case 2: // left
		{
			if (x > MIN_X)
			{
				x = x - 10;
				// draw head
				putImage(x,y,8,8,snakeheadH,1,0);
			}
			break;
		}

		case 3: // up
		{
			if (y > MIN_Y)
			{
				y = y - 10;
				// draw head
				putImage(x,y,8,8,snakeheadV,0,0);
			}
			break;
		}
	}
	updateBody1();

	//remove duplicate head
	if(snakeBody1[1][1] != 0)
	{
		fillRectangle(snakeBody1[1][1], snakeBody1[2][1], 8, 8, RGBToWord(0,200,20));
	}

	
	
}

void move2(int direction)
{
	

	switch(direction)
	{
		case 0: // right
		{
			if (x2 < MAX_X)
			{
				x2 = x2 + 10;
				// draw head
				putImage(x2,y2,8,8,snakehead2H,0,0);
			}
			break;
		}

		case 1: // down
		{
			if (y2 < MAX_Y)
			{
				y2 = y2 + 10;
				// draw head
				putImage(x2,y2,8,8,snakehead2V,0,1);
			}
			break;
		}

		case 2: // left
		{
			if (x2 > MIN_X)
			{
				x2 = x2 - 10;
				// draw head
				putImage(x2,y2,8,8,snakehead2H,1,0);
			}
			break;
		}

		case 3: // up
		{
			if (y2 > MIN_Y)
			{
				y2 = y2 - 10;
				// draw head
				putImage(x2,y2,8,8,snakehead2V,0,0);
			}
			break;
		}
	}

	updateBody2();

	//remove duplicate head
	if(snakeBody2[1][1] != 0)
	{
		fillRectangle(snakeBody2[1][1], snakeBody2[2][1], 8, 8, RGBToWord(255,180,21));
	}

	
	
}

void updateBody1()
{
	short endofSnake1 = 0;

	// find end of snake (first 0 element)
	while( snakeBody1[0][endofSnake1] != 0)
	{
		endofSnake1++;
	}
	endofSnake1--;

	// score count printer
	printNumber((endofSnake1 - 2), 10, 5, RGBToWord(0xff, 0xff, 0), 0);


	// delete trailing body piece
	// do not write if x = 0... i.e. if x has yet to be assigned... snake not fully in yet
	if (snakeBody1[1][endofSnake1] != 0)
	{
		fillRectangle(snakeBody1[1][endofSnake1],snakeBody1[2][endofSnake1],8,8,0);
	}
	


	// propagate movement from back to front -- keep track of where body pieces are
	for (int j = endofSnake1; j > 0; j--)
	{
		snakeBody1[1][j] = snakeBody1[1][j-1];
		snakeBody1[2][j] = snakeBody1[2][j-1];
	}

	snakeBody1[1][0] = x;
	snakeBody1[2][0] = y;

	checkforCollision1(endofSnake1);
	
}

void updateBody2()
{
	short endofSnake2 = 0;

	// find end of snake (first 0 element)
	while( snakeBody2[0][endofSnake2] != 0)
	{
		endofSnake2++;
	}
	endofSnake2--;

	// score count printer
	printNumber((endofSnake2-2), 80, 5, RGBToWord(0xff, 0xff, 0), 0);


	// delete trailing body piece
	// do not write if x = 0... i.e. if x has yet to be assigned... snake not fully in yet
	if (snakeBody2[1][endofSnake2] != 0)
	{
		fillRectangle(snakeBody2[1][endofSnake2],snakeBody2[2][endofSnake2],8,8,0);
	}
	


	// propagate movement from back to front -- keep track of where body pieces are
	for (int j = endofSnake2; j > 0; j--)
	{
		snakeBody2[1][j] = snakeBody2[1][j-1];
		snakeBody2[2][j] = snakeBody2[2][j-1];
	}

	snakeBody2[1][0] = x2;
	snakeBody2[2][0] = y2;

	checkforCollision2(endofSnake2);
	
}

void checkforCollision2(int endofSnake2)
{
	short endofSnake1 = 0;

	// find end of snake (first 0 element)
	while( snakeBody1[0][endofSnake1] != 0)
	{
		endofSnake1++;
	}
	endofSnake1--;

	for (int i = 1; i <= endofSnake2; i++)
	{
		// checks if this snake is hitting itself
		if (x2 == snakeBody2[1][i])
		{
			if(y2 == snakeBody2[2][i])
			{
				for (int j = 0; j <= endofSnake2; j++)
				{
					fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, 255);
				}
				gameOver2(endofSnake2);
				break;
			}
		}

		// checks if snake 1 is colliding with this snake
		if (x == snakeBody2[1][i])
		{
			if(y == snakeBody2[2][i])
			{
				for (int j = 0; j <= endofSnake2; j++)
				{
					fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, 255);
				}
				gameOver(endofSnake1);
				break;
			}
		}
	}
	

	// nom nom
	if (x2 == appleX && y2 == appleY)
	{
		//eat the apple, get longer
		snakeBody2[0][endofSnake2 + 1] = 1;
		appleEaten = 1;
		spawnApple();
	}

}
void checkforCollision1(int endofSnake1)
{
	

	for (int i = 1; i <= endofSnake1; i++)
	{
		// checks if this snake is colliding with itself
		if (x == snakeBody1[1][i])
		{
			if(y == snakeBody1[2][i])
			{
				for (int j = 0; j <= endofSnake1; j++)
				{
					fillRectangle(snakeBody1[1][j], snakeBody1[2][j], 8, 8, 255);
				}

				gameOver(endofSnake1);
				break;
			}
		}
	}

	//checks if snake 2 exists
	if(snakeBody2[0][0] != 0)
	{
		short endofSnake2 = 0;

		// find end of snake 2 (first 0 element)
		while( snakeBody2[0][endofSnake2] != 0)
		{
			endofSnake2++;
		}
		endofSnake2--;

		for (int i = 1; i <= endofSnake1; i++)
		{
			//checks if snake 2 is colliding with this snake

			if (x2 == snakeBody1[1][i])
			{
				if(y2 == snakeBody1[2][i])
				{
					for (int j = 0; j <= endofSnake1; j++)
					{
						fillRectangle(snakeBody2[1][j], snakeBody2[2][j], 8, 8, 255);
					}
					gameOver2(endofSnake2);
					break;
				}
			}
		}
	}

	// nom nom
	if (x == appleX && y == appleY)
	{
		//eat the apple, get longer
		snakeBody1[0][endofSnake1 + 1] = 1;
		appleEaten = 1;
		spawnApple();
	}

}

void drawBorder()
{
	fillRectangle(0,0,128,14,0);
	fillRectangle(0,14,128,154,RGBToWord(255,8,0));
	fillRectangle(MIN_X, MIN_Y, MAX_X - 2, MAX_Y - 2, 0);
}

void spawnApple()
{
	uint8_t i = 0;

	// seed the random number generator with a range 0 – large number
    srand(milliseconds * x * y);

	
	//x: 10 - 110
	//y: 20 - 140
	appleX = ((rand() % 11) + 1) * 10;
	appleY = ((rand() % 13) + 2) * 10;

	

	//check if apple is trying to spawn inside snake
	while(snakeBody1[0][i] != 0 || snakeBody2[0][i] != 0)
	{
		if ((snakeBody1[1][i] == appleX && snakeBody1[2][i] == appleY) || (snakeBody2[1][i] == appleX && snakeBody2[2][i] == appleY))
		{
			i = -1;

			srand(milliseconds * x * y);

			//x: 10 - 110
			//y: 20 - 140
			appleX = ((rand() % 11) + 1) * 10;
			appleY = ((rand() % 13) + 2) * 10;
		}

		i++;
	}

	putImage(appleX, appleY, 8, 8, apple, 0, 0);

	

	eputs("Apple Pos: (");
	printDecimal(appleX);
	eputs(",");
	printDecimal(appleY);
	eputs(")\n");
	

}

void initSysTick(void)
{
	SysTick->LOAD = 48000;
	SysTick->CTRL = 7;
	SysTick->VAL = 10;
	__asm(" cpsie i "); // enable interrupts
}
void SysTick_Handler(void)
{
	milliseconds++;
}
void initClock(void)
{
// This is potentially a dangerous function as it could
// result in a system with an invalid clock signal - result: a stuck system
        // Set the PLL up
        // First ensure PLL is disabled
        RCC->CR &= ~(1u<<24);
        while( (RCC->CR & (1 <<25))); // wait for PLL ready to be cleared
        
// Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
// inserted into Flash memory interface
				
        FLASH->ACR |= (1 << 0);
        FLASH->ACR &=~((1u << 2) | (1u<<1));
        // Turn on FLASH prefetch buffer
        FLASH->ACR |= (1 << 4);
        // set PLL multiplier to 12 (yielding 48MHz)
        RCC->CFGR &= ~((1u<<21) | (1u<<20) | (1u<<19) | (1u<<18));
        RCC->CFGR |= ((1<<21) | (1<<19) ); 

        // Need to limit ADC clock to below 14MHz so will change ADC prescaler to 4
        RCC->CFGR |= (1<<14);

        // and turn the PLL back on again
        RCC->CR |= (1<<24);        
        // set PLL as system clock source 
        RCC->CFGR |= (1<<1);
}
void delay(volatile uint32_t dly)
{
	uint32_t end_time = dly + milliseconds;
	while(milliseconds != end_time)
		__asm(" wfi "); // sleep
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR &~(3u << BitNumber*2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber*2); // set pull-up bit
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
	*/
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectange defined by x,y,w,h
	uint16_t x2,y2;
	x2 = x1+w;
	y2 = y1+h;
	int rvalue = 0;
	if ( (px >= x1) && (px <= x2))
	{
		// ok, x constraint met
		if ( (py >= y1) && (py <= y2))
			rvalue = 1;
	}
	return rvalue;
}

void setupIO()
{
	RCC->AHBENR |= (1 << 18) + (1 << 17); // enable Ports A and B
	display_begin();
	pinMode(GPIOB,4,0);
	pinMode(GPIOB,5,0);
	pinMode(GPIOA,8,0);
	pinMode(GPIOA,11,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,8);
}