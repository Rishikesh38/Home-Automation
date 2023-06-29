/*
 * Uart2.c
 *
 *  Created on: Dec 10, 2021
 *      Author: 40018933
 */


#include "main.h"
#include "Uart2.h"
#include "stdbool.h"
#include "string.h"
#include "Led.h"
#include "stdlib.h"
#include "Controller.h"


/*
 * To provide clock for UART2 we need to set the Bit position of Uart2 (17th bit) in RCC APB1 peripheral clock enable register.
 */
#define UART2_CLOCK_ENABLE 17

/*
 * To provide clock for GPIOA we need to set the Bit position of GPIOA (0th bit) in RCC AHB1 peripheral clock register.
 */
#define GPIOA_CLOCK_ENABLE 0

/*
 * Macro used to set the bit position to 1
 */
#define SET 1

/*
 * In GPIO port mode register we need to select a mode. So we need alternate function mode for the serial communication protocol.
 * The pin configuration to use alternate function is 10 i.e. 2
 */
#define ALTERNATE_FUNCTION_MODE 2

/*
 * We are using PA2 GPIO pin as UART Transmit. So the bit position of PA2 pin start from 4th bit.
 */
#define PA2 4

/*
 * We are using PA3 GPIO pin as UART Receive. So the bit position of PA3 pin start from 6th bit.
 */
#define PA3 6

/*
 * In GPIO  port output speed register we need to select a speed. So we choose very high speed
 * The pin configuration to use very high speed is 11 i.e. 3
 */
#define VERY_HIGH_SPEED 3

/*
 * In GPIO alternate function low register we need to select Alternate function according to the peripheral used.
 * We used UART2 so we need to use AF7 -> 0111 i.e 7
 */
#define AF7 7

/*
 * We are using PA2 GPIO pin as UART Transmit. So the bit position of PA2(AFRL2) pin start from 8th bit.
 */
#define AFRL2 8

/*
 * We are using PA3 GPIO pin as UART Receive. So the bit position of PA3(AFRL3) pin start from 12th bit.
 */
#define AFRL3 12

/*
 * Enable the UART by writing the UE bit in USART_CR1(Control register 1) register to 1
 * The bit position of UART Enable is 13
 */
#define UART_ENABLE 13

/*
 * Program the M bit in USART_CR1 to define the word length
 * Bit position of M is 12
 * Put 0 in that bit position for using 8 data bits
 */
#define WORD_LENGTH 12

/*
 * The value of DIV_FRACTION is 13
 * We can find that value by using Baud rate formula mentioned in the reference manual
 */
#define DIV_FRACTION 13

/*
 * The value of DIV_MANTISSA is 22
 * We can find that value by using Baud rate formula mentioned in the reference manual
 */
#define DIV_MANTISSA 22

/*
 * The bit position of DIV_FRACTION starts from 0th bit
 * We need to put 13 from 0th bit
 */
#define DIV_FRACTION_BIT_POSITION 0

/*
 * The bit position of DIV_MANTISSA starts from 4th bit
 * We need to put 22 from 4th bit
 */
#define DIV_MANTISSA_BIT_POSITION 4

/*
 * Set the TE bit in USART_CR1 for transmitter and Set RE bit in USART_CR1 for receiver
 */
#define TRANSMIT_ENABLE 3
#define RECEIVE_ENABLE  2

/*
 * The bit position of transmission complete is 6 in Status register
 */
#define TRANSMISSION_COMPLETE 6

/*
 * The bit position of  Read data register not empty is 5 in status register
 */
#define READ_DATA_REGISTER_NOT_EMPTY 5

/*
 * To generate a interrupt for receiving data we need to set the bit position of RXNEIE in CR1 register
 * The bit position of RXNEIE is 5
 */

/*
 * The bit position of transmit data register empty is 7 in status register
 */
#define TRANSMIT_DATA_REGISTER_EMPTY 7

#define RXNEIE 5

/*
 * To generate a interrupt for transferring data we need to set the bit position of TXEIE in CR1 register
 * The bit position of TXEIE is 7
 */
#define TXEIE 7

#define False 0

#define True 1

bool CommandComplete = False;
static bool SecModeOn = False;
static uint8_t IntensityHighLevel= 0xff;
static uint8_t IntensityLowLevel = 0;
static uint16_t MaxDistance=400;
static uint32_t TimeOut=0;
uint8_t BufferIndex=0;
char buffer[10];
bool FirstChar = False;

/** @brief Function Used to return High level Intensity.
 */
uint8_t GetHighLevelIntenisty(void)
{
	return IntensityHighLevel;
}

/** @brief Function Used to return low level Intensity.
 */
uint8_t GetLowLevelIntensity(void)
{
	return IntensityLowLevel;
}

/** @brief Function Used to return Maximum distance.
 */
uint16_t GetMaxDistance(void)
{
	return MaxDistance;
}

/** @brief Function Used to return Security Mode On flag.
 */
bool GetSecurityMode(void)
{
	return SecModeOn;
}

/** @brief Function Used to return Time out value.
 */
uint32_t GetTimeOut(void)
{
	return TimeOut*1000000;
}

/** @brief To Configure all the register required for UART Protocol.
 */
void Uart2Config(void)
{
	//1.Enable the UART Clock and GPIO Clock

	RCC->APB1ENR |= (SET<<UART2_CLOCK_ENABLE); // bit 17 which is UART Clock Enable is SET now.
	RCC->AHB1ENR |= (SET<<GPIOA_CLOCK_ENABLE); // bit 0 which is GPIOA clock enable is SET now.

	//2.Configure the UART pins for alternate functions (set GPIO port mode register, GPIO port output speed register, AFR)

	GPIOA->MODER |= (ALTERNATE_FUNCTION_MODE<<PA2);
	GPIOA->MODER |= (ALTERNATE_FUNCTION_MODE<<PA3);
	GPIOA->OSPEEDR |= (VERY_HIGH_SPEED<<PA2) | (VERY_HIGH_SPEED<<PA3);
	GPIOA->AFR[0] |= (AF7<<AFRL2) | (AF7<<AFRL3);

	//3.Enable the UART by writing the UE bit in USART_CR1(Control register 1) register to 1

	USART2->CR1 |= (SET<<UART_ENABLE); // UE bit position in USART Control register1 is 13.

	//4. Program the M bit in USART_CR1 to define the word length

	USART2->CR1 &= ~(SET<<WORD_LENGTH);

	/* 5.Select the desired baud rate using the USART_BRR register
	 *  Formula to Calculate Baud Rate is Tx/Rx baud = Clock Frequency/16*USARTDIV
	 */

	USART2->BRR |= (DIV_FRACTION<<DIV_FRACTION_BIT_POSITION) | (DIV_MANTISSA<<DIV_MANTISSA_BIT_POSITION);

	//6. Set the TE bit in USART_CR1 for transmitter and Set RE bit in USART_CR1 for receiver

	USART2->CR1 |= (SET<<RECEIVE_ENABLE) | (SET<<TRANSMIT_ENABLE);

	//7. Enable RXEIE interrupt in UART CR1 register

	USART2->CR1 |= (SET<<RXNEIE);

	//8. Interrupt for UART2 on NVIC side

	NVIC_EnableIRQ(USART2_IRQn);
}

/** @brief IRQHandler function is called whenever a command is received from user.
 * @details Verification of the command is done. And if it is the right command then its stored.
 */
void USART2_IRQHandler(void)
{
	if(USART2->SR & (SET<<READ_DATA_REGISTER_NOT_EMPTY))
	{
		char temp = USART2->DR;
		if(FirstChar!= true)
		{
			BufferIndex=0;
			if(temp == '$')
			{
				FirstChar = true;
				 buffer[BufferIndex++] = temp;
			}
		}
		else
		{
			if(temp == '#')
			{
				FirstChar = False;
				buffer[BufferIndex++] = temp;
				buffer[BufferIndex++] = '\0';
				for (uint8_t j=0; j<BufferIndex; j++)
				{
					USART2->DR = buffer[j];
					while(!(USART2->SR & (SET<<TRANSMISSION_COMPLETE)));
				}
				BufferIndex=0;
				CommandComplete = True;
			}
			else if(BufferIndex>=10)
			{
				BufferIndex=0;
				FirstChar = False;
			}
			else
			{
				//Drop the Received command if $ received in between
				if(temp == '$')
				{
					BufferIndex=0;
				}
				buffer[BufferIndex++] = temp;
			}
		}
	}
}

/** @brief Parsing of the stored command is done
 * @details Inside this function it compares the received command and does the respective operation
 */
void ParseCommand()
{
	const char SecurityModeOnCmd[] = "$SMON#";
	const char SecurityOffCmd[] = "$SMOFF#";
	const char HighLevelIntensityCmd[] = "$SIH";
	const char LowLevelIntensityCmd[] = "$SIL";
	const char MaxDistanceCmd[] = "$MD";
	const char TimeOutCmd[]= "$TO";
	char IntensityHigh[3];
	char IntensityLow[2];
	char MaximumDistance[3];
	char TimeOutLimit[3];

	if(CommandComplete)
	{
		if(!strcmp(buffer,SecurityModeOnCmd))
		{
			SecModeOn = True;
		}
		else if(!strcmp(buffer,SecurityOffCmd))
		{
			SecModeOn = False;
		}
		else if(!strncmp(buffer,HighLevelIntensityCmd,4))
		{
			for(int BufferIndex=0; BufferIndex<=2;BufferIndex++)
			{
				IntensityHigh[BufferIndex] = buffer[BufferIndex+5];
			}
			IntensityHighLevel = atoi(IntensityHigh);
			NotifyNewIntensity();
		}
		else if(!strncmp(buffer,LowLevelIntensityCmd,4))
		{
			for(int BufferIndex=0; BufferIndex<=1;BufferIndex++)
			{
				IntensityLow[BufferIndex] = buffer[BufferIndex+5];
			}
			IntensityLowLevel = atoi(IntensityLow);
			NotifyNewIntensity();
		}
		else if(!strncmp(buffer,MaxDistanceCmd,3))
		{
			for(int BufferIndex=0;BufferIndex<=2;BufferIndex++)
			{
				MaximumDistance[BufferIndex] = buffer[BufferIndex+4];
			}
			MaxDistance = atoi(MaximumDistance);
		}
		else if(!strncmp(buffer,TimeOutCmd,3))
		{
			for(int BufferIndex=0;BufferIndex<=2;BufferIndex++)
			{
				TimeOutLimit[BufferIndex] = buffer[BufferIndex+4];
			}
			TimeOut = atoi(TimeOutLimit);
			NotifyNewTimeOut();
		}
	}
	CommandComplete = False;
}


