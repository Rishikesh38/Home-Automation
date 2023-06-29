/*
 * Delay.h
 *
 *  Created on: Dec 21, 2021
 *      Author: 40018933
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_
#include<stdint.h>
void TIM6Config (void);
void TIM6_DAC_IRQHandler(void);
void DelayMicroSec (uint16_t us);
void DelayMilliSec (uint16_t ms);
#endif /* INC_DELAY_H_ */
