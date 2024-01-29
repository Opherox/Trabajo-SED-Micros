/*
 * CS43L22_I2C.h
 *
 *  Created on: Jan 20, 2024
 *      Author: david
 */

#ifndef INC_CS43L22_I2C_H_
#define INC_CS43L22_I2C_H_

#include "stm32f4xx_hal.h"

static uint8_t registros_volumen[]={
		0x00, //0db
		0x01, //0.5db
		0x02, //1db
		0x03, //1.5db
		0x04, //2db
		0x05, //2.5db
		0x06, //3db
		0x07, //3.5db
		0x08, //4db
		0x09, //4.5db
		0x0A, //5db
		0x0B, //5.5db
		0x0C, //6db
		0x0D, //6.5db
		0x0E, //7db
		0x0F, //7.5db
		0x10, //8db
		0x11, //8.5db
		0x12, //9db
		0x13, //9.5db
		0x14, //10db
		0x15, //10.5db
		0x16, //11db
		0x17, //11.5db
		0x18, //12db
};

void CS43L22_Init(I2C_HandleTypeDef *i2c);
void CS43L22_ON();
void CS43L22_OFF();
void CS43L22_Volume(uint16_t volumen);

#endif /* INC_CS43L22_I2C_H_ */
