/*
 * Led.h
 *
 *  Created on: Dec 21, 2021
 *      Author: 40018933
 */

#ifndef INC_LED_H_
#define INC_LED_H_
#include<stdint.h>
void GPIOConfig(void);
void SecurityModeOn();
void SecurityModeOff();
extern uint8_t Count;
void LedOn();
void LedOff();
void IntensityConfig(void);
void UpdateIntensity(uint32_t Intensity);


#endif /* INC_LED_H_ */
