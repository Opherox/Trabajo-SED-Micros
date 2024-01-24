/*
 * CS43L22_I2C.h
 *
 *  Created on: Jan 20, 2024
 *      Author: david
 */

#ifndef INC_CS43L22_I2C_H_
#define INC_CS43L22_I2C_H_

#include "stm32f4xx_hal.h"

void CS43L22_Init(I2C_HandleTypeDef *i2c);
void CS43L22_ON();
void CS43L22_OFF();


#endif /* INC_CS43L22_I2C_H_ */
