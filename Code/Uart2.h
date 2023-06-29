/*
 * Uart2.h
 *
 *  Created on: Dec 10, 2021
 *      Author: 40018933
 */

#ifndef INC_UART2_H_
#define INC_UART2_H_
#include<stdint.h>
#include "stdbool.h"
void Uart2Config (void);
uint16_t GetMaxDistance(void);
bool GetSecurityMode(void);
uint8_t GetHighLevelIntenisty(void);
uint8_t GetLowLevelIntensity(void);
uint32_t GetTimeOut(void);
void USART2_IRQHandler(void);
void ParseCommand();
#endif /* INC_UART2_H_ */
