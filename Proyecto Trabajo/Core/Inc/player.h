/*
 * audio.h
 *
 *  Created on: Jan 23, 2024
 *      Author: lcastedo
 */

#ifndef INC_PLAYER_H_
#define INC_PLAYER_H_

#include <stm32f4xx_hal.h>
#include <ff.h>

#include <stdbool.h>

void PLAYER_init(I2C_HandleTypeDef *hi2c1, I2S_HandleTypeDef *hi2s3);
bool PLAYER_isPlaying(void);
bool PLAYER_play(const char *songFPath);
void PLAYER_stop(void);
void PLAYER_setVolume(unsigned vol);

#endif /* INC_PLAYER_H_ */
