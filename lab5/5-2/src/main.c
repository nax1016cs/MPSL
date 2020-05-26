#include "stm32l476xx.h"

//TODO: define your gpio pin
/*
#define X0
#define X1
#define X2
#define X3
#define Y0
#define Y1
#define Y2
#define Y3
unsigned int x_pin[4] = {X0, X1, X2, X3};
unsigned int y_pin[4] = {Y0, Y1, Y2, Y3};
*/

const unsigned int Table[4][4] = {	{1, 2, 3, 10},
					{4, 5, 6, 11},
					{7, 8, 9, 12},
					{15, 0, 14, 13}};


//These functions inside the asm file
extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);

/* TODO: initial keypad gpio pin, X as output and Y as input */
void keypad_init() {
	// SET keypad gpio OUTPUT //
		RCC->AHB2ENR = RCC->AHB2ENR|0x2;
		//Set PA8,9,10,12 as output mode
		GPIOA->MODER= GPIOA->MODER&0xFDD5FFFF;
		//set PA8,9,10,12 is Pull-up output
		GPIOA->PUPDR=GPIOA->PUPDR|0x1150000;
		//Set PA8,9,10,12 as medium speed mode
		GPIOA->OSPEEDR=GPIOA->OSPEEDR|0x1150000;
		//Set PA8,9,10,12 as high
		GPIOA->ODR=GPIOA->ODR|10111<<8;

	// SET keypad gpio INPUT //
	    //Set PB5,6,7,9 as INPUT mode
		GPIOB->MODER=GPIOB->MODER&0xFFF303FF;
		//set PB5,6,7,9 is Pull-down input
		GPIOB->PUPDR=GPIOB->PUPDR|0x8A800;
		//Set PB5,6,7,9 as medium speed mode
		GPIOB->OSPEEDR=GPIOB->OSPEEDR|0x45400;
}

/* TODO: scan keypad value return:
>=0: key pressedvalue
-1: no keypress */
char keypad_scan() {
	while (1) {
		int k = 0;
		unsigned int flag_keypad = 0, flag_debounce = 0;
		flag_keypad=GPIOB->IDR&10111<<5;
		if (flag_keypad!=0) {
			k = 20000;
		}
		while (k != 0) {
			flag_debounce=GPIOB->IDR&10111<<5;
			k--;
		}
		if (flag_debounce != 0) {
			for (int i=0; i<4; i++){
				//scan keypad from first column
				int position_c = i+8;
				if (i==3)
					position_c++;
				//set PA8,9,10,12(column) low and set pin high from PA8
				GPIOA->ODR=(GPIOA->ODR&0xFFFFE8FF)|1<<position_c;

				for (int j=0; j<4; j++) {
					//read input from first row
					int position_r = j+5;
					if (j == 3)
						position_r++;

					int flag_keypad_r=GPIOB->IDR&1<<position_r;
					if (flag_keypad_r != 0)
						display(Table[j][i]);
				}
			}
		} else {
			max7219_send(1, 15);
			max7219_send(2, 15);
		}
		GPIOA->ODR=GPIOA->ODR|10111<<8; //set PA8,9,10,12(column) high
	}


}


int display(int num) {
	unsigned int digit = num % 10;
	max7219_send(1, digit);
	num /= 10;
	if (num)
		max7219_send(2, num);
	else
		max7219_send(2, 15);
	return 0;
}

void main() {
	unsigned int student_id = 616026;
	GPIO_init();
	max7219_init();
	keypad_init();
	keypad_scan();
}