/*
 * Controller.c
 *
 *  Created on: Jan 18, 2022
 *      Author: 40018933
 */
#include "Controller.h"
#include "main.h"
#include "Uart2.h"
#include "Sensor.h"
#include "Led.h"
#include "stdbool.h"

/*
 * To enable the timer3 clock in RCC registers
 */
#define TIMER_ENABLED 1

/*
 * To enable the clock
 */
#define CLOCK_ENABLE 1

/*
 * The prescalar value used
 */
#define PRESCALAR_VALUE 83

/*
 * To enable the interrupt
 */
#define INTERRUPT_ENABLED 0

/*
 * Interrupt occurs when timer overflows
 */
#define UPDATE_REQUEST_SOURCE 2

uint32_t PastDistance = 0;
uint32_t CheckTimeOut = 0;
uint32_t FalseDistance = 0xffffffff;

static bool isTimerRunning(void);
bool movementDetected(uint32_t CurrentDist, uint32_t PreviousDistance);
static void restartTimer(void);
static bool timeOutExpired(uint32_t TimeOutValue);
static void stopTimer(void);
static void startTimer(void);
static void Intensity_Calc(void);

/** @brief To Configure all the register required for time out feature.
 */
void TimeOutConfig(void)
{
	//1. Enable Clock for TIM7
	RCC->APB1ENR |= (CLOCK_ENABLE << TIMER_ENABLED);

	//2. Set the prescalar and the ARR
	TIM3->PSC = PRESCALAR_VALUE;

	//Interrupt enabled
	TIM3->DIER |= (SET << INTERRUPT_ENABLED);

	//Interrupt occurs only when counter overflows
	TIM3->CR1 |= (CLOCK_ENABLE << UPDATE_REQUEST_SOURCE);

	NVIC_EnableIRQ(TIM3_IRQn);
}

/** @brief Checks which mode need to be activated and also checks for timeout.
 * @details This function is called in main.c and here we check the mode,
 * maximum distance, time out value and act accordingly.
 */
void Check_Mode(void)
{
	bool SecurityModeTurnOn = GetSecurityMode();
	uint16_t MaxDist = GetMaxDistance();
	uint32_t TimeOutValue = GetTimeOut();
	uint32_t Dist = GetDistance();

	if (SecurityModeTurnOn && Dist <= MaxDist)
	{
		SecurityModeOn();
	}
	else if (!(SecurityModeTurnOn))
	{
		SecurityModeOff();
	}
	if ((!(SecurityModeTurnOn)) && Dist <= MaxDist)
	{
		if (movementDetected(Dist, FalseDistance))
		{
			Intensity_Calc();
			FalseDistance = 0xffffffff;
			if (isTimerRunning())
			{
				if (movementDetected(Dist, PastDistance))
				{
					PastDistance = Dist;
					restartTimer();
				}
				else if (timeOutExpired(TimeOutValue))
				{
					LedOff();
					stopTimer();
					PastDistance = 0;
					FalseDistance = Dist;
				}
			}
			else
			{
				Intensity_Calc();
				LedOn();
				if (TimeOutValue > 0)
				{
					PastDistance = Dist;
					startTimer();
				}
			}
		}
	}
	else
	{
		LedOff();
		stopTimer();
	}
}

/** @brief Time out feature timer overflow is handled.
 */
void TIM3_IRQHandler(void)
{
	if (TIM3->SR & (1 << 0))
	{
		CheckTimeOut += 65535;
	}
	TIM3->SR &= ~(1 << 0);
}

/** @brief To check whether the time out feature timer is running or not.
 */
bool isTimerRunning(void)
{
	bool returnValue = false;
	if (TIM3->CR1 & (1 << 0))
	{
		returnValue = true;
	}
	return returnValue;
}

/** @brief To check whether the object is moving or not.
 */
bool movementDetected(uint32_t CurrentDist, uint32_t PreviousDistance)
{
	bool retVal = false;
	if (!(CurrentDist > (PreviousDistance - 3)
			&& CurrentDist < (PreviousDistance + 3)))
	{
		retVal = true;
	}
	return retVal;
}

/** @brief To restart the time out feature timer.
 */
void restartTimer(void)
{
	TIM3->CR1 &= ~(1 << 0);
	TIM3->CNT = 0;
	CheckTimeOut = 0;
	TIM3->CR1 |= (1 << 0);
}

/** @brief To check whether the time out value set by the user is expired or not.
 */
bool timeOutExpired(uint32_t TimeOutValue)
{
	bool retValue = false;
	if (CheckTimeOut + TIM3->CNT >= TimeOutValue)
	{
		retValue = true;
	}
	return retValue;
}

/** @brief Function Used to stop the time out feature timer.
 */
void stopTimer(void)
{
	TIM3->CR1 &= ~(1 << 0);
	TIM3->CNT = 0;
	CheckTimeOut = 0;
}

/** @brief Function Used to start the time out feature timer.
 */
void startTimer(void)
{
	TIM3->CR1 |= (1 << 0);
}

/** @brief Whenever user enters new timeout value the timer needs to be stopped
 * and starts when led glows
 */
void NotifyNewTimeOut(void)
{
	if (isTimerRunning())
	{
		stopTimer();
	}
}

/** @brief Calculates the intensity by using distance.
 */
void Intensity_Calc(void)
{
	uint32_t Intensity = 0;
	Intensity = ((100 - (GetDistance() * 100) / GetMaxDistance()));
	if (Intensity > GetHighLevelIntenisty())
	{
		Intensity = GetHighLevelIntenisty();
	}
	else if (Intensity < GetLowLevelIntensity())
	{
		Intensity = GetLowLevelIntensity();
	}
	UpdateIntensity(Intensity);
}

/** @brief Calculates intensity whenever the user enters new high or low levels of intensity.
 */
void NotifyNewIntensity(void)
{
	if (isTimerRunning())
	{
		Intensity_Calc();
	}
}
