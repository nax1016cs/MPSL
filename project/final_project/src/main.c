#include "stm32l476xx.h"


//These functions inside the asm file
extern void GPIO_init();
extern void max7219_init();
extern void max7219_send(unsigned char address, unsigned char data);
extern void max7219_init2();
extern void max7219_send2(unsigned char address, unsigned char data);

#define SET_REG(REG, SELECT, VAL) { ((REG)=((REG)&(~(SELECT))) | (VAL)); };
#define GPIO_PIN_0  ((uint16_t) 0x0001)
#define GPIO_PIN_1  ((uint16_t) 0x0002)
#define GPIO_PIN_2  ((uint16_t) 0x0004)
#define GPIO_PIN_3  ((uint16_t) 0x0008)
#define TIM_COUNTERMODE_UP 0
#define TIM_ARR_VAL 99U
#define TIM_PSC_VAL 39999U

int SCORE_A = 0;
int SCORE_B = 0;
int TIME_LEFT = 9000;
int state = 0;
int time;
int sound_count = 0;

void reset_SCORE_TIME() {

 for (int i = 2; i <= 6; i++) {
  max7219_send(i, 15);
 }
 max7219_send(1, 0);
 max7219_send(7, 0);
 max7219_send(8, 15);
 for (int i = 1; i <= 8; i++) {
  max7219_send2(i, 15);
 }
}

void display_SCORE_A() {
 int tmp_score = SCORE_A;
 if (tmp_score == 0) {
  max7219_send(7, 0);
  return;
 }
 for (int i = 7; i <= 8; i++) {
  int digit = tmp_score % 10;
  if (tmp_score == 0) digit = 15;
  max7219_send(i, digit);
  tmp_score /= 10;
 }
}

void display_SCORE_B() {
 int tmp_score = SCORE_B;
 if (tmp_score == 0) {
  max7219_send(1, 0);
  return;
 }
 for (int i = 1; i <= 2; i++) {
  int digit = tmp_score % 10;
  if (tmp_score == 0) digit = 15;
  max7219_send(i, digit);
  tmp_score /= 10;
 }
}

void display_SEC(int sec) {
 for (int i = 1; i <= 2; i++) {
  int digit = sec % 10;
  max7219_send2(i, digit);
  sec /= 10;
 }
}

void display_MIN(int min) {
 for (int i = 5; i <= 6; i++) {
  int digit = min % 10;
  max7219_send2(i, digit);
  min /= 10;
 }
}

void display_TIME() {
 int tmp_time = TIME_LEFT;
 if (tmp_time < 0) tmp_time = 0;

 int sec = tmp_time / 100;
 display_SEC(sec%60);

 int min = sec / 60;
 display_MIN(min);
}


int ReadGPIO(GPIO_TypeDef *GPIOX, uint16_t GPIO_PIN_Y) {
 return GPIOX->IDR & GPIO_PIN_Y;
}


void init_gpio(){
 RCC->AHB2ENR = RCC->AHB2ENR|0x7;
// GPIOC->MODER=GPIOC->MODER&0xFFFFFF00;
// //set PC0,1,2,3 is Pull-down input
// GPIOC->PUPDR=GPIOC->PUPDR|0xAA;
// //Set PC0,1,2,3 as medium speed mode
// GPIOC->OSPEEDR=GPIOC->OSPEEDR|0x55;

// GPIOC->MODER=GPIOC->MODER&0xFFFDD500;
// //set PC0,1,2,3 is Pull-down input, PC4,5,6,8 as output
// GPIOC->PUPDR=GPIOC->PUPDR|0x115AA;
// //Set PC0,1,2,3 as medium speed mode, Set PC4,5,6,8 as medium speed mode
// GPIOC->OSPEEDR=GPIOC->OSPEEDR|0x11555;
// //Set PC4,5,6,8 as high
// GPIOC->ODR=GPIOC->ODR|00000<<4;

 GPIOC->MODER=GPIOC->MODER&0xFF55D500;
 //set PC0,1,2,3 is Pull-down input, PC4,5,6,8 as output
 GPIOC->PUPDR=GPIOC->PUPDR|0x5515AA;
 //Set PC0,1,2,3 as medium speed mode, Set PC4,5,6,8 as medium speed mode
 GPIOC->OSPEEDR=GPIOC->OSPEEDR|0x3D1555;
 //Set PC4,5,6,8 as high
 GPIOC->ODR=GPIOC->ODR|00000<<4;

 GPIOA->MODER=GPIOA->MODER&0xFFFFFF00;
 GPIOA->MODER=GPIOA->MODER|0x55;
 //set PC0,1,2,3 is Pull-down input, PC4,5,6,8 as output
// GPIOA->PUPDR=GPIOA->PUPDR&0xFFFFFF00;
// GPIOA->PUPDR=GPIOA->PUPDR|0xAA;
 //Set PC0,1,2,3 as medium speed mode, Set PC4,5,6,8 as medium speed mode
 GPIOA->OSPEEDR=GPIOA->OSPEEDR&0xFFFFFF00;
 GPIOA->OSPEEDR=GPIOA->OSPEEDR|0x55;
 //Set PC4,5,6,8 as high
}

void Timer_init(TIM_TypeDef *timer){
 RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
 SET_REG(TIM2->CR1, TIM_CR1_DIR, TIM_COUNTERMODE_UP);
 TIM2->ARR = (uint32_t)TIM_ARR_VAL;
 TIM2->PSC = (uint32_t)TIM_PSC_VAL;
 TIM2->EGR = 0x0001;
}


void Game(TIM_TypeDef* timer) {
 TIM2->CR1 |= TIM_CR1_CEN;

 int pre_val = TIM_ARR_VAL;
 int n = 0;
 //int cur_time = 0;

 int tmpA = SCORE_A;
 int tmpB = SCORE_B;
 int tmp_time = TIME_LEFT;
 int state23_count = 100;
 int state0_count = 0;

 while(1){
	 if(sound_count > 0)
		 sound_count--;
	 if (sound_count == 0)
		 GPIOA->ODR=GPIOA->ODR&11111111111111000;

  if (state == 0) {
   // Waiting for start
	  state0_count++;
	  state0_count %= 8000;

	  if (SCORE_A > SCORE_B) {
		  if (state0_count == 4000) {
			  max7219_send(7, 15);
			  max7219_send(8, 15);
		  } else if (state0_count == 0) {
			  display_SCORE_A();
		  }
	  } else if (SCORE_A < SCORE_B) {
		  if (state0_count == 4000) {
			  max7219_send(1, 15);
		  	  max7219_send(2, 15);
		  } else if (state0_count == 0) {
		  	display_SCORE_B();
		  }
	  }
   if(!ReadGPIO(GPIOC, GPIO_PIN_2)) {
    GPIOC->ODR=GPIOC->ODR&0<<6;
   } else {
    GPIOC->ODR=GPIOC->ODR|1<<6;
   }
   if(!ReadGPIO(GPIOC, GPIO_PIN_3)) {
    GPIOC->ODR=GPIOC->ODR&0<<8;
   } else {
    GPIOC->ODR=GPIOC->ODR|1<<8;
   }

   if(!ReadGPIO(GPIOC, GPIO_PIN_2) && !ReadGPIO(GPIOC, GPIO_PIN_3)) {
    GPIOA->ODR=GPIOA->ODR&11111111111111000;
    GPIOA->ODR=GPIOA->ODR|1;
    sound_count = 100;
    //GPIOC->ODR=GPIOC->ODR&0<<9;
    state = 1;
    SCORE_A = 0;
    SCORE_B = 0;
    TIME_LEFT = 9000;
    time = 0;
    reset_SCORE_TIME();
   }
  } else if (state == 1) {
   // Playing

	  //GPIOA->ODR=GPIOA->ODR&11111111111111000;
	  GPIOA->ODR = 0;
   if (/*n < 100 * TIME_SEC &&*/ time != TIM2->CNT) {
    ++n;
    TIME_LEFT--;
    if (TIME_LEFT <= 0) {

     GPIOA->ODR=GPIOA->ODR&11111111111111000;
     GPIOA->ODR=GPIOA->ODR|1;
     sound_count = 100;
     //GPIOC->ODR=GPIOC->ODR&0<<11;

     state = 0;
     state0_count = 0;
    }
    time = TIM2->CNT;
    display_TIME();
    if (state == 0) {
    int temp = 1000;
     //while(temp--);
     //continue;
    }
   }
   // detect score of a
   if(!ReadGPIO(GPIOC, GPIO_PIN_0)){
	   GPIOA->ODR=GPIOA->ODR&11111111111111000;
	   GPIOA->ODR=GPIOA->ODR|10;
	    sound_count = 100;


	SCORE_A++;
    state = 2;
    state23_count = 5000;


    //GPIOC->ODR=GPIOC->ODR&0<<10;

    GPIOC->ODR=GPIOC->ODR|1<<4;
   }
   // detect score of b
   else if(!ReadGPIO(GPIOC, GPIO_PIN_1)){
	   GPIOA->ODR=GPIOA->ODR&11111111111111000;
	   GPIOA->ODR=GPIOA->ODR|10;
	    sound_count = 100;

    SCORE_B++;
    state = 3;
    state23_count = 5000;


    //GPIOC->ODR=GPIOC->ODR&0<<10;

    GPIOC->ODR=GPIOC->ODR|1<<5;
   }

  } else if (state == 2) {
   // Goal for A and wait for resume
   display_SCORE_A();
   if(ReadGPIO(GPIOC, GPIO_PIN_2)) {
    state23_count--;
    //GPIOC->ODR=GPIOC->ODR&1011<<4;

    GPIOC->ODR=GPIOC->ODR&1111111110111111;
    GPIOC->ODR=GPIOC->ODR|1<<4; // Don't know why!!!!!!!!
   } else {
    state23_count = 5000;
    GPIOC->ODR=GPIOC->ODR|1<<6;
   }
   if (state23_count <= 0) {
    state = 1;
    //GPIOC->ODR=GPIOC->ODR&00000<<4;
    //GPIOC->ODR=GPIOC->ODR&1111111000001111;
    GPIOC->ODR= 0;
   }
  } else if (state == 3) {
   // Goal for B and  wait for resume
   display_SCORE_B();
   if(ReadGPIO(GPIOC, GPIO_PIN_3)) {
    state23_count--;
    //GPIOC->ODR=GPIOC->ODR&01111<<4;
    GPIOC->ODR=GPIOC->ODR&1111111011111111;
    GPIOC->ODR=GPIOC->ODR|1<<5; // Don't know why!!!!!!!!
   } else {
    state23_count = 5000;
    GPIOC->ODR=GPIOC->ODR|1<<8;
   }
   if (state23_count <= 0) {
    state = 1;
    //GPIOC->ODR=GPIOC->ODR&00000<<4;
    //GPIOC->ODR=GPIOC->ODR&1111111000001111;
    GPIOC->ODR= 0;
   }
  }

 }

}

void main() {
 //unsigned int student_id = 616026;
 //unsigned int student_id2 = 516097;
 GPIO_init();
 max7219_init();
 max7219_init2();

 init_gpio();
 Timer_init(TIM2);

 GPIOA->ODR=GPIOA->ODR&11111111111111000;
 time = 0;

 reset_SCORE_TIME();


 Game(TIM2);


 //display();
}