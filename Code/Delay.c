/*
 * Delay.c
 *
 *  Created on: Dec 21, 2021
 *      Author: 40018933
 */


#include "main.h"
#include "Led.h"

/*
 * To enable the clock for the timer6
 */
#define CLOCK_ENABLE 1

/*
 * The bit position of timer6 in RCC register is 4
 */
#define TIMER6 4

/*
 * The ARR value used here
 */
#define ARR_VALUE 83

/*
 * The bit position of URS in CR1 register
 */
#define URS 2

/*
 * To enable the interrupt
 */
#define INTERRUPT_ENABLE 0

/*
 * To enable the Counter
 */
#define COUNTER_ENABLE 0


void TIM6Config (void)
{
//1. Enable the Timer Clock
	RCC->APB1ENR |= (CLOCK_ENABLE<<TIMER6);

	//2. Set the prescalar and the ARR
	TIM6->PSC = RESET;

	TIM6->ARR = ARR_VALUE;

	//3. Set the interrupt for only overflow

	// Bit position of URS(Update Request Source) is 2
	TIM6->CR1 = (SET<<URS);

	//4. Enable the update interrupt

	// Bit position of update interrupt enable is 0 in DMA/Interrupt enable register
	TIM6->DIER = (SET<<INTERRUPT_ENABLE);

	NVIC_EnableIRQ( TIM6_DAC_IRQn);
}

int Ticks = 0;

/** @brief Provides delay in microseconds.
 * @param us microsecond delay value.
 */
void DelayMicroSec (uint16_t us)
{
	TIM6->CR1 |= (SET<<COUNTER_ENABLE);
	if(Ticks>=us)
	{
	Count++;
	TIM6->CR1 &= ~(1<<0);
	Ticks=0;
	}
}

/** @brief Provides delay in milliseconds.
 * @param ms millisecond delay value.
 */
void DelayMilliSec (uint16_t ms)
{
	TIM6->CR1 |= (SET<<COUNTER_ENABLE);
	if(Ticks>= (ms*1000))
	{
	 Count++;
	 TIM6->CR1 &= ~(1<<0);
	 Ticks=0;
	}
}

/** @brief Increments the tick value.
 */
void TIM6_DAC_IRQHandler(void)
{
	//check the interrupt.
	if(TIM6->SR & (1<<0));
	TIM6->SR &= ~(1<<0);
	Ticks++;
}


