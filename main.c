//===================================================================================
//Data: 10/12/2021
//Autora: Marina Lourenço Borges
//Descrição: Lab_3 - Microcontrolados S23
//===================================================================================


/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
  Program 7.5, example 7.6

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */




#include "PLL.h" 						//inicia o temporizador em 80 MHz
#include "Timer0.h"		
#include "SysTick.h"
#include "tm4c123gh6pm.h"		
#include <stdint.h>

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF0       (*((volatile uint32_t *)0x40025001))
#define PD3       (*((volatile uint32_t *)0x40007020))


int duration;
unsigned long In;  		// Input para PF4
unsigned long Out; 		// Visor para PD3
unsigned long OutGreen;	 	// Visor para LED green
unsigned long OutBlue; 		// Visor para LED blue
unsigned long OutRed; 		// Visor para LED red
unsigned long OutRed2; 		// Visor para LED red
unsigned long OutLed; 		// Visor para LED red
unsigned int flash_Green = 0;	// Looping para LED Verde
unsigned int flash_Red = 0;	// Looping para LED Vermelho
unsigned int flash_Red2 = 0;	// Looping para LED Vermelho frequência 2.5 Hz
unsigned int flash_All = 0;	// Looping para todos os LEDs


static uint32_t i = 0;		//variável i do exercício


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode
void PortF_Init(void);  			//protótipo da função de inic.
void Delay (void);
void Delay_1 (void);

//inicialização do portF
void PortF_Init(void){
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;    // F clock
  delay = SYSCTL_RCGC2_R;          // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;          // allow changes to PF4-0 
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PORTF
  GPIO_PORTF_PCTL_R = 0x00000000;  // configure PORTF as GPIO
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF0, PF4 input, PF1-PF3 output
  GPIO_PORTF_AFSEL_R &= ~0x1F;     // disable alt funct 
  GPIO_PORTF_PUR_R = 0x11;         // enable weak pull-up on PF4 and PF0
  GPIO_PORTF_DEN_R |= 0x1F;        // enable digital I/O
}


//inicialização do portD
//PD3 como saída
void PortD_Init(void){    
	SYSCTL_RCGCGPIO_R |= 0x00000008; // (a) activate clock for port D          
  GPIO_PORTD_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port D
  GPIO_PORTD_CR_R = 0x08;           // allow changes to PD3
  GPIO_PORTD_DIR_R |=  0x08;    // output on PD3
  GPIO_PORTD_AFSEL_R &= ~0x1F;  //     disable alt funct on PD
  GPIO_PORTD_DEN_R |= 0x1F;     //     enable digital I/O on PD3
  GPIO_PORTD_PCTL_R &= ~0x000FFFFF; // configure PD as GPIO
  GPIO_PORTD_AMSEL_R = 0;       //     disable analog functionality on PD
}



void Timer0A_Handler(void){
  Timer0_Init(800000000);
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;	// acknowledge TIMER0A timeout
	TIMER0_CTL_R = 0x0;
	
}

int main(void){ volatile unsigned long delay;

  PLL_Init();                      // bus clock at 80 MHz
	PortF_Init();										 // initialize PortF
	PortD_Init();
  Timer0_Init(800000000); 				// initialize timer1 (0,1 Hz)
	SysTick_Init();
  EnableInterrupts();
	
  while(1){
	switch(i)
    {
				//ESTADO 0
        case 0:
						GPIO_PORTF_DATA_R = 0x00;
						PD3 = GPIO_PORTD_DATA_R&0x08; //leitura da PD3
						Timer0A_Handler();
				
				//ESTADO 1
        case 1:
					//pisca led azul com 2,5 Hz
					GPIO_PORTF_DATA_R = 0x04;  // 0b0000_0100 LED acende Azul
					OutBlue = GPIO_PORTF_DATA_R&0x04; //Saída para o Led Azul
					SysTick_Wait(16000000); // Delay 2,5Hz
					GPIO_PORTF_DATA_R = 0x00;  // 0b0000_0000 LED off
					OutBlue = GPIO_PORTF_DATA_R&0x04; //Saída para o Led Azul
					SysTick_Wait(16000000);  // Delay 2,5Hz
				
					GPIO_PORTD_DATA_R = 0x00;  //Inicializa a PD3 como LOW 0x0000_0000
					Out = GPIO_PORTD_DATA_R&0x08;//Indicar o que tem na porta PD3

				Timer0A_Handler();
				
				case 2:	
				Timer0_Init(3200000);
				if((GPIO_PORTF_DATA_R&0x01)==0x01)
				{case 3:
					i = 1;
					while (flash_Green < 5) 
						{
							GPIO_PORTF_DATA_R = 0x08;  // 0b0000_1000 LED acende Verde
							OutGreen = GPIO_PORTF_DATA_R&0x08; // Indicar nível no LED green
							SysTick_Wait(3200000);
							GPIO_PORTF_DATA_R = 0x00;  // 0b0000_0000 LED is black
							OutGreen = GPIO_PORTF_DATA_R&0x08; // Indicar nível no LED green
							SysTick_Wait(3200000);	
							flash_Green = flash_Green + 1;
						}	
				} 
				else {	
					case 4:
						while (flash_Red < 2)
							{// Estado 4 - LED red piscar 2 vezes
								GPIO_PORTF_DATA_R = 0x02;  // 0b0000_0010 LED is red
								OutRed = GPIO_PORTF_DATA_R&0x02; // Indicar nível no LED red
								SysTick_Wait(3200000);
								GPIO_PORTF_DATA_R = 0x00;  // 0b0000_0000 LED is black
								OutRed = GPIO_PORTF_DATA_R&0x02; // Indicar nível no LED red
								SysTick_Wait(3200000);
								flash_Red = flash_Red + 1;
							}
					}	
				Timer0A_Handler();
					
        case 5:		

						while (flash_Red2 < 2) 
			{			GPIO_PORTF_DATA_R = 0x02;  // 0b0000_0010 LED is red 2.5 Hz
						SysTick_Wait(16000000); // Delay 2,5Hz
						OutRed2 = GPIO_PORTF_DATA_R&0x02;
						GPIO_PORTF_DATA_R = 0x00;  // 0b0000_0000 LED is black
						SysTick_Wait(16000000); // Delay 2,5Hz
						OutRed2 = GPIO_PORTF_DATA_R&0x00;
						flash_Red2 = flash_Red2 + 1;
			}
						
				Timer0A_Handler();
			
        case 6:		
					//pisca em sequência os 3 LEDS
					//repete 4x essa ação
							GPIO_PORTD_DATA_R = 0x08;  // 0x0000_0000 PD3 is high
							Out = GPIO_PORTD_DATA_R&0x08;// indicar o que tem na porta PD3
							SysTick_Wait(16000000);
							
							while (flash_All < 4)
							{
								GPIO_PORTF_DATA_R = 0x02;  // 0b0000_0010 LED Vermelho
								SysTick_Wait(3200000);
								GPIO_PORTF_DATA_R = 0x04;  // 0b0000_0100 LED Azul
								SysTick_Wait(3200000);
								GPIO_PORTF_DATA_R = 0x08;  // 0b0000_1000 LED Verde
								SysTick_Wait(3200000);
								flash_All = flash_All + 1;
							}							
							i = 0;
		}
  }
}
