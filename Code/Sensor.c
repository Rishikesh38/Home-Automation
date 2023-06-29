/*
 * Sensor.c
 *
 *  Created on: Dec 29, 2021
 *      Author: 40018933
 */

#include "main.h"
#include "Sensor.h"
#include "stdbool.h"
#include "Uart2.h"

#define TRIGGER_PULSE_HIGH 50
#define TRIGGER_PULSE_LOW  61000
#define MAX_LIMIT 4294967295

static uint32_t Counts = 20;
static uint32_t RaisingEdgeStamp=0;
static uint32_t FallingEdgeStamp=0;
static uint32_t TimeStamp=0xffffffff;
static uint32_t Distance=0xffffffff;

/*
 * To provide clock for GPIOB we need to set the Bit position of GPIOB in RCC clock enable register.
 */
#define GPIOB_CLOCK_ENABLE 1

/*
 * PA10 is trigger pin and its starting bit position is 20
 */
#define TRIGGER_PIN 20

/*
 * PA11 is echo pin and its starting bit position was 22
 */
#define ECHO_PIN 22

/*
 * In GPIO port mode register we need to select a mode. So we need alternate function mode
 * The pin configuration to use alternate function is 10 i.e. 2
 */
#define ALTERNATE_FUNCTION_MODE 2

/*
 * In GPIO  port output speed register we need to select a speed. So we choose high speed
 * The pin configuration to use very high speed is 10 i.e. 2
 */
#define HIGH_SPEED 2

/*
 * In GPIO alternate function high register we need to select Alternate function according to the peripheral used.
 * We used TIM2 so we need to use AF1 -> 0001 i.e 1
 */
#define AF1 1

/*
 * We are using PA2 GPIO pin as UART Transmit. So the bit position of PA2(AFRL2) pin start from 8th bit.
 */
#define AFRH10 8

/*
 * We are using PA3 GPIO pin as UART Receive. So the bit position of PA3(AFRL3) pin start from 12th bit.
 */
#define AFRH11 12

/*
 * Enabling clock for the TIM2 in RCC registers
 */
#define TIMER2_CLOCK_ENABLE 0

/*
 * Enabling the channel
 */
#define INPUT_CHANNEL 1

/*
 * Bit position in CCMR2 register to enable channel
 */
#define CAPTURE_COMPARE4_SELECTION 8

/*
 * Bit position in CCMR2 register for selection of filter
 */
#define INPUT_CAPTURE_FILTER 12

/*
 * Selected the filter
 */
#define FILTER 3

/*
 * Selected the polarity of CC
 */
#define CAPTURE_COMPARE_OUTPUT_POLARITY 13

/*
 * Selected the polarity of CC
 */
#define CAPTURE_COMPARE_OUTPUT_POLARITYY 15

/*
 * To enable the capture compare
 */
#define CAPTURE_ENABLED 1

/*
 * Bit to be set in CCER
 */
#define CC4E 12

/*
 * Bit to be set in DIER
 */
#define CC4IE 4

/*
 * To enable the interrupt
 */
#define INTERRUPT_ENABLE 1

/*
 * To be set in EGR for generation of capture
 */
#define CAPTURE_COMPARE_GENERATION 4

/*
 * To generate an interrupt in EGR
 */
#define CAPTURE_EVENT_OCCUR 1

/*
 * Value of prescalar
 */
#define PRESCALAR_VALUE 84-1

/*
 * To enable output compare mode
 */
#define OUTPUT_COMPARE_MODE 4

/*
 * To toggle the pin when interrupt generated
 */
#define PIN_TOGGLE 3

/*
 * to enable the CCOE
 */
#define CAPTURE_COMPARE_OUTPUT_ENABLE 8

/*
 * The bit position of CCIE in DIER
 */
#define CC3IE 3

/*
 * To enable the Counter
 */
#define COUNTER_ENABLE 0

/*
 * Bit position in EGR
 */
#define CC3G 3

/** @brief Function Used to return distance of the person or object.
 */
uint32_t GetDistance(void)
{
	return Distance;
}

/** @brief To configure all the registers required for sensor module.
 */
void SensorConfig(void)
{
	//1. Configure the clock for GPIOB

	 RCC->AHB1ENR |= (SET<<GPIOB_CLOCK_ENABLE);

	//2. Configure the GPIOB registers

	// Pin PB10(TRIG) is used as GPIO Alternate Function Mode

	GPIOB->MODER |= (ALTERNATE_FUNCTION_MODE<<TRIGGER_PIN);

	// Pin PB11(ECHO) is used as GPIO Alternate Function Mode

	GPIOB->MODER |= (ALTERNATE_FUNCTION_MODE<<ECHO_PIN);

	// Pin PB10 & PB11 are set to High Speed

	GPIOB->OSPEEDR |= (HIGH_SPEED<<TRIGGER_PIN) | (HIGH_SPEED<<ECHO_PIN);

	//Setting PB10 & PB11 to TIM2

	GPIOB->AFR[1] |= (AF1<<AFRH10) | (AF1<<AFRH11);

	//3. //a) Configuring a Timer for Echo Pin (PB11) - Input Capture Mode - PB11 - TIM2_CH4	(Echo)

	//Enable the clock for TIM2

	RCC->APB1ENR |= (SET<<TIMER2_CLOCK_ENABLE);

	//CC4 channel is configured as input, IC4 is mapped on TI4

	TIM2->CCMR2 |= (INPUT_CHANNEL<<CAPTURE_COMPARE4_SELECTION);

	//To select CC4 channel filter

	TIM2->CCMR2 |= (FILTER<<INPUT_CAPTURE_FILTER);

	//Select the edge for CC4 channel

	// set 1,3bits for detecting both edges

	TIM2->CCER |= (SET<<CAPTURE_COMPARE_OUTPUT_POLARITY) | (SET<<CAPTURE_COMPARE_OUTPUT_POLARITYY);

	//Enable the capture into the counter register for CC4 channel

	TIM2->CCER |= (CAPTURE_ENABLED<<CC4E);

	//Interrupt Enabled

	TIM2 ->DIER |= (INTERRUPT_ENABLE<<CC4IE);

	//Event or Interrupt is generated when a Capture is done

	TIM2->EGR |= (CAPTURE_EVENT_OCCUR<<CAPTURE_COMPARE_GENERATION);

	// Set the prescalar to 84-1 to get the time delay of 1Micro sec

	TIM2->PSC = PRESCALAR_VALUE;

	//TIM2->ARR = 0Xffffffff, 4294967295 max value of ARR is reset value so no need to set it

//	b) Configuring a Timer for Trigger Pin (PB10) - Output Compare Mode - PB10 - TIM2_CH3 (Trig)

		// CC3 channel is configured as output

		//TIM2->CCMR2 &= ~(1<<0);

		// CC3 channel is configured as output

		//TIM2->CCMR2 &= ~(1<<1);

		//Toggle when CNT and TIM2_CCRx value are same for CC3 channel, Toggle->011(3)

		TIM2->CCMR2 |= (PIN_TOGGLE<<OUTPUT_COMPARE_MODE);

		// signal is output on the corresponding output pin

		TIM2->CCER |= (CAPTURE_ENABLED<<CAPTURE_COMPARE_OUTPUT_ENABLE);

		// Polarity Active High

		//TIM2->CCER &= ~(1<<9);

		//Interrupt Enabled

		TIM2 ->DIER |= (INTERRUPT_ENABLE<<CC3IE);

		//Event or Interrupt generated when a CNT and TIM2_CCRx value are same

		TIM2->EGR |= (SET<<CC3G);

		//Counter Enabled

		TIM2->CR1 |= (SET<<COUNTER_ENABLE);

		TIM2->CCR3 = Counts;

	NVIC_EnableIRQ( TIM2_IRQn);
}

/** @brief Output compare and input capture modes.
 * @details A 10microsec pulse is sent to sensor trigger pin which is done by output compare mode,
 * the echo pulse is received and time stamp is noted in input capture mode
 */
void TIM2_IRQHandler(void)
{
	//Check if the interrupt
	if(TIM2->SR & (1<<3))
	{
		// need to check the GPIO status and if pin is high then we need 15 microseconds
		if(GPIOB->IDR & (1<<10))
		{
			Counts= TIM2->CNT + TRIGGER_PULSE_HIGH;

			//Timer overflow is handled
			if(Counts > MAX_LIMIT)
			{
				Counts -= MAX_LIMIT;
				TIM2->CCR3 = Counts;
			}
			else
			{
				TIM2->CCR3 = Counts;
			}
		}

		//Pin low then we need 61 Milliseconds
		else
		{
			Counts = TIM2->CNT + TRIGGER_PULSE_LOW;

			//Timer overflow is handled
			if(Counts > MAX_LIMIT)
			{
				Counts -= MAX_LIMIT;
				TIM2->CCR3 = Counts;
			}
			else
			{
				TIM2->CCR3 = Counts;
			}
		}
		//Clear the interrupt
		TIM2->SR &= ~(1<<3);
	}
		//Check if the interrupt is because of Input Capture Mode
		if(TIM2->SR & (1<<4))
		{
			//Check the GPIO is high. If its high then its raising edge
			if(GPIOB->IDR & (1<<11))
			{
			RaisingEdgeStamp = TIM2->CCR4;
			}

			else
			{
			FallingEdgeStamp = TIM2->CCR4;

			//If raising edge==0 then leave calculation
			if(RaisingEdgeStamp != 0)
			{
			if(FallingEdgeStamp>RaisingEdgeStamp)
				{
					TimeStamp=(FallingEdgeStamp - RaisingEdgeStamp);
				}
			else if(RaisingEdgeStamp>FallingEdgeStamp)
				{
					TimeStamp=(0xffffffff-RaisingEdgeStamp)+FallingEdgeStamp;
				}
			FallingEdgeStamp = 0;
			RaisingEdgeStamp = 0;
		    }
		}
			//Clear the interrupt
			TIM2->SR &= ~(1<<4);
	}
}

/** @brief To calculate the distance
 * @details Need to calculate the distance of the object or person from the time stamp value
 * Distance= speed*time i.e. speed of sound is 343 m/s
 */
void Disc_Calc(void)
{
	//If time stamp is greater than 35 milliseconds then no presence of person
  if(TimeStamp<=35000)
  {
	  Distance = TimeStamp/58;
  }
}






