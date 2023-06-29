/*
 * Controller.h
 *
 *  Created on: Jan 18, 2022
 *      Author: 40018933
 */

#ifndef INC_CONTROLLER_H_
#define INC_CONTROLLER_H_
void Check_Mode(void);
void TimeOutConfig (void);
void NotifyNewTimeOut(void);
void TIM3_IRQHandler(void);
void NotifyNewIntensity(void);
#endif /* INC_CONTROLLER_H_ */
