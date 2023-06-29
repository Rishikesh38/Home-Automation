/*
 * Sensor.h
 *
 *  Created on: Dec 29, 2021
 *      Author: 40018933
 */

#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_
#include "stdbool.h"

void SensorConfig(void);
void TIM2_IRQHandler(void);
void Disc_Calc(void);
uint32_t GetDistance(void);
#endif /* INC_SENSOR_H_ */
