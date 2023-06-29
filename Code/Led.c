/*
 * Led.c
 *
 *  Created on: Dec 21, 2021
 *      Author: 40018933
 */

#include "Led.h"
#include "Delay.h"
#include "main.h"
#include "Sensor.h"

/*
 * To enable the clock in RCC registers
 */
#define GPIOD_CLOCK_ENABLE 3

/*
 * Output mode selected for GPIO pin PD14
 */
#define GPIO_OUTPUT_MODE 1

/*
 * bit position of Pin PD14 is 28 in MODER Register
 */
#define PD14 28

/*
 * bit position of Pin PD12 is 28 in GPIO SPEED Register
 */
#define PD12 24

/*
 * Set the Speed register in GPIO registers into high speed
 */
#define HIGH_SPEED 2

/*
 * The Alternate function register for timer
 */
#define AFRH 2

/*
 * Bit position for AFRH12 for PD12 is 16 in GPIOD->AFR register
 */
#define AFRH12 16

/*
 * Clock Enable
 */
#define CLOCK_ENABLE 1

/*
 * Bit position to enable clock for TIM4 is 2
 */
#define TIMER4 2

/*
 * To change the intensity pulse width modulation mode is used
 */
#define PWM 6

/*
 * Compare Mode is where the mode is set(OC1M)
 */
#define COMPARE_MODE 4

/*
 * To enable the preload
 */
#define PRELOAD 3

/*
 * Set the Auto Reload
 */
#define AUTO_RELOAD 7

/*
 * Update generation bit useful to generate an interrupt
 */
#define UG 0

/*
 * To set the polarity to active high
 */
#define ACTIVE_HIGH 1

/*
 * To enable the capture compare
 */
#define CAPTURE_ENABLE 0

/*
 * The value of the prescalar used
 */
#define PRESCALAR 84-1

/*
 * The value of the ARR used
 */
#define ARR_VALUE 1000

/*
 * Bit position of Timer counter in CR1 register
 */
#define TIMER_COUNTER 0

/*
 * To put the pin in alternate function mode
 */
#define ALTERNATE_FUNCTION_MODE 2

uint8_t Count =1;

/** @brief All the registers related to GPIO are configured.
 *
 */
void GPIOConfig(void)
{
	RCC->AHB1ENR |= (SET<<GPIOD_CLOCK_ENABLE); //Enable the clock for GPIOD Register


	GPIOD->MODER |= (GPIO_OUTPUT_MODE<<PD14);  //GPIO Output mode for PD14

//	GPIOD->OTYPER &= ~(1<<14); //GPIO Output push pull mode for PD14
//
//	GPIOD->OTYPER &= ~(1<<12); //GPIO Output push pull mode for PD12

	GPIOD->OSPEEDR |= ((HIGH_SPEED<<PD14) | (HIGH_SPEED<<PD12)); //GPIO Speed register set to fast speed for PD14 and PD12
}

/** @brief All the register related to intensity control are configured.
 */
void IntensityConfig(void)
{
	//2. Configure the GPIO Registers
	GPIOD->AFR[1] |= (AFRH<<AFRH12); //TIM4 is in AFRH->AFR2

	//3. Configure the timer for PD12(TIM4_CH1)
	//Enable Clock for TIM4
	RCC->APB1ENR |= (CLOCK_ENABLE<<TIMER4);

	//PWM1 Mode Enabled
	TIM4->CCMR1 |= (PWM<<COMPARE_MODE);

	//Output compare 1 preload enable
	TIM4->CCMR1 |= (SET<<PRELOAD);

	//ARR Setting
	TIM4->CR1 |= (SET<<AUTO_RELOAD);

	 //Set the UG Bit
	TIM4->EGR |= (SET<<UG);

	//Polarity to active high
	TIM4->CCER &= ~(SET<<ACTIVE_HIGH);

	//OC1 signal is output on the corresponding output pin(PD12)
	TIM4->CCER |= (SET<<CAPTURE_ENABLE);

	// Making sure psc*arr = 84000 such that we get freq=1khz so time=1millisec
	TIM4->PSC = PRESCALAR;

	//TIM4_CNT Overflows for 100 So now if CCR4 is 50 the DutyCycle is 50
	TIM4->ARR = ARR_VALUE;
}

/** @brief To Turn On the security mode.
 *  @details Blinking of Red Led happens in security mode function
 */
void SecurityModeOn()
{
	if(Count==1)
	{
	// Led On
	GPIOD->BSRR |= (SET<<14);
	DelayMilliSec(500);
	}
	if(Count==2)
	{
	// Led Off
	GPIOD->BSRR |= (1<<14) <<16;
	DelayMilliSec(500);
	}
	if(Count>=3)
	{
		Count=1;
	}
}

/** @brief To Turn Off the security mode.
 *  @details Red Led is turned off
 */
void SecurityModeOff()
{
	GPIOD->BSRR |= (SET<<14) <<16; // Reset the RedLed Pin
}

/** @brief Turn On the Led.
 * @details Green Led will be turned on
 * timer(counter) used for intensity control is enabled.
 */
void LedOn()
{
	// Reseting 24th bit to remove Output mode
	GPIOD->MODER &= ~(SET<<PD12);

	// Alternate function mode for PD12(Green Led)
	GPIOD->MODER |= (ALTERNATE_FUNCTION_MODE<<PD12);

	// Intensity Counter Enabled
	TIM4->CR1 |= (SET<<TIMER_COUNTER);
}

/** @brief Turn Off the Led.
 * @details Green and red Led will be turned off
 * timer(counter) used for intensity control is disabled.
 */
void LedOff()
{
	// Intensity Counter Disabled
	TIM4->CR1 &= ~(SET<<TIMER_COUNTER);

	// Reseting 25th bit to remove Alternate function mode
	GPIOD->MODER &= ~(SET<<25);

	// GPIO output mode for PD12
	GPIOD->MODER |= (SET<<PD12);

	//Green Led PD12 will be Reset
	GPIOD->BSRR |= (SET<<12) <<16;

	// Red Led Off
	GPIOD->BSRR |= (SET<<14) <<16;
}

/** @brief Updates the intensity.
 * @param Intensity
 */
void UpdateIntensity(uint32_t Intensity)
{
	TIM4->CCR1 = Intensity*10;
}
