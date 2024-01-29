/*
 * volume.h
 *
 *  Created on: Jan 21, 2024
 *      Author: ferna
 */

#ifndef INC_VOLUME_H_
#define INC_VOLUME_H_
#include "stm32f4xx_hal.h"

void oled_Init();
//extern char *songPlaying;
uint16_t changeVolume(ADC_HandleTypeDef *hadc1, I2C_HandleTypeDef *hi2c1);

#endif /* INC_VOLUME_H_ */
