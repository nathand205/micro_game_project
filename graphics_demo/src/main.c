#include <stm32f031x6.h>
#include "display.h"

#define MAX_SIZE 30
#define MIN_Y 20
#define MAX_Y 131
#define MIN_X 10
#define MAX_X 110


void initSound(void);
void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);

volatile uint32_t milliseconds;


const uint16_t snake[]=
{
	40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,40224,
};

void move(int direction);
void updateBody();
void checkforCollision1(int);

void mainMenu();
void creditScroll();

int x = 50;
int y = 50;
int snakeBody[3][MAX_SIZE] = {0};
int applesEaten = 1;
void drawBorder();
void sneyk1P();
void playNote(uint32_t freq);

int main()
{
	initClock();
	initSysTick();
	setupIO();

	

	mainMenu();
	
	return 0;
}

void playNote(uint32_t freq)
{
// Counter is running at 4MHz 
// Lowest possible frequency = 4000000/65536 = 61 Hz approx
    if (freq == 0)
    {
        TIM2->CR1 &= ~(1 << 0); // disable the counter
    }
	TIM2->ARR = (uint32_t)4000000/((uint32_t)freq); 
	TIM2->CCR4= TIM2->ARR/2;	
	TIM2->CNT = 0; // set the count to zero initially
	TIM2->CR1 |= (1 << 0); // and enable the counter
}

void initSound()
{    
// Sound will be produced by sending the output from Timer 2, channel 4 out PB1
    pinMode(GPIOB,1,2); // select alternative function for PB1
    GPIOB->AFR[0] &= ~( (0b1111) << 1*4); // zero out alternate function bits
    GPIOB->AFR[0] |= ( (0b0101) << 1*4); // set required alternate function bits (AF5)
    
    RCC->APB1ENR |= (1 << 0);  // enable Timer 2
    TIM2->CR1 = 0; // Set Timer 2 to default values
	TIM2->CCMR2 = (1 << 14) + (1 << 13);
	TIM2->CCER |= (1 << 12);
	TIM2->PSC = 3;
	TIM2->ARR = 1UL;
	TIM2->CCR4 = TIM2->ARR/2;	
	TIM2->CNT = 0;
}

void sneyk1P()
{
	short p1Direction = 0; // 0 - R, 1 - D, 2 - L, 3 - U
	short currentCycle = 1;

	drawBorder();
	printTextX2("SCORE:", 0, 0, RGBToWord(0xff,0xff,0), 0);

	snakeBody[0][0] = 1; //place head
	snakeBody[0][1] = 1;
	snakeBody[0][2] = 1;
	snakeBody[0][3] = 1;

	while(1)
	{
		
		
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
		
		if(currentCycle == 8)
		{
			move(p1Direction);

			currentCycle = 1;
		}
		else
		{
			currentCycle++;
		}

		delay(50);
	}
}

void creditScroll()
{
	
	fillRectangle(0,0,128,154,0);

	for (int i = 140; i >= 0; i--)
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
	fillRectangle(0,0,128,154,0);
	printTextX2("__________", 5, 8, RGBToWord(0xff,0xff,0), 0);
	printTextX2("SNEYK", 35, 0, RGBToWord(0xff,0xff,0), 0);
}

void mainMenu()
{
	int currentSelection = 0;
	fillRectangle(0,0,128,154,0);
	printTextX2("__________", 5, 8, RGBToWord(0xff,0xff,0), 0);
	printTextX2("SNEYK", 35, 0, RGBToWord(0xff,0xff,0), 0);
	

	while(1)
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

			delay(200);
		}
		if ( (GPIOA->IDR & (1 << 8)) == 0) // up pressed
		{
			if(currentSelection == 0)
			{
				currentSelection = 2;
			}
			else
			{
				currentSelection--;
			}

			delay(200);
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
					playNote(500);			
					sneyk1P();	
				}


				break;
			}

			case 1:
			{
				printTextX2("1 PLAYER  ", 10, 40, RGBToWord(0xff,0xff,0), 0);
				printTextX2("2 PLAYER >", 10, 70, 255, 0);
				printTextX2("CREDITS  ", 10, 100, RGBToWord(0xff,0xff,0), 0);
				break;
			}

			case 2:
			{
				printTextX2("1 PLAYER  ", 10, 40, RGBToWord(0xff,0xff,0), 0);
				printTextX2("2 PLAYER  ", 10, 70, RGBToWord(0xff,0xff,0), 0);
				printTextX2("CREDITS >", 10, 100, 255, 0);
				if ((GPIOB->IDR & (1 << 4)) == 0) // right pressed
				{		
					playNote(500);	
					creditScroll();	
				}
				
				break;
			}
		}
		
	}

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

void move(int direction)
{

	switch(direction)
	{
		case 0: // right
		{
			if (x < MAX_X)
			{
				x = x + 10;
				// hinverted=0;
			}
			break;
		}

		case 1: // down
		{
			if (y < MAX_Y)
			{
				y = y + 10;
			}
			break;
		}

		case 2: // left
		{
			if (x > MIN_X)
			{
				x = x - 10;
			}
			break;
		}

		case 3: // up
		{
			if (y > MIN_Y)
			{
				y = y - 10;
			}
			break;
		}
	}

	putImage(x,y,8,8,snake,0,0);

	updateBody();
}

void updateBody()
{
	short endOfSnake = 0;

	// find end of snake (first 0 element)
	while( snakeBody[0][endOfSnake] != 0)
	{
		endOfSnake++;
	}
	endOfSnake--;

	// delete trailing body piece
	if (snakeBody[1][endOfSnake] != 0)
	{
		fillRectangle(snakeBody[1][endOfSnake],snakeBody[2][endOfSnake],8,8,0);
	}
	


	// propagate movement from back to front -- keep track of where body pieces are
	for (int j = endOfSnake; j > 0; j--)
	{
		snakeBody[1][j] = snakeBody[1][j-1];
		snakeBody[2][j] = snakeBody[2][j-1];
	}

	snakeBody[1][0] = x;
	snakeBody[2][0] = y;

	checkforCollision1(endOfSnake);
	
}

void checkforCollision1(int lastFilled)
{
	
	for (int i = 1; i <= lastFilled; i++)
	{
		if (x == snakeBody[1][i])
		{
			if(y == snakeBody[2][i])
			{
				for (int j = 0; j <= lastFilled; j++)
				{
					fillRectangle(snakeBody[1][j], snakeBody[2][j], 8, 8, 255);
				}

				break;
			}
		}
	}

	


}

void drawBorder()
{
	
	fillRectangle(0,0,128,154,RGBToWord(255,8,0));
	fillRectangle(MIN_X, MIN_Y, MAX_X - 2, MAX_Y - 2, 0);
}