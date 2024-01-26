/*
 * volume.c
 *
 *  Created on: Jan 21, 2024
 *      Author: ferna
 */

/* Includes ------------------------------------------------------------------*/
#include "volume.h"
#include "ssd1306.h"
#include "fonts.h"
#include "FileSystem.h"

int AppState;

void oled_Init()
{
	SSD1306_Init();
}
void displayAppState(int state)
{
	AppState = state;
}
uint16_t changeVolume(ADC_HandleTypeDef *hadc1, I2C_HandleTypeDef *hi2c1)
{
    /* TODO: eliminar macro cuando se use hi2c1 */
    UNUSED(hi2c1);

    uint16_t adcval;
	uint16_t value;
	uint16_t volume;
	const uint16_t adcval_MAX = 4095;
	uint16_t rectangle_x = 5;
	uint16_t rectangle_y = 20;
	uint16_t rectangle_width = 118;
	uint16_t rectangle_height = 20;
	//ADC
	HAL_ADC_Start(hadc1);
	if(HAL_ADC_PollForConversion(hadc1, HAL_MAX_DELAY)== HAL_OK){
		adcval = HAL_ADC_GetValue(hadc1);
	}
	HAL_ADC_Stop(hadc1);

	value = (uint16_t)((adcval * rectangle_width)/adcval_MAX);	//From 0 to 118
	volume = (uint16_t)((adcval * 100)/adcval_MAX);				//From 0 to 100

	//Display OLED
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(5, 0);
	SSD1306_Puts("Volumen", &Font_11x18, 1);
	SSD1306_GotoXY(5,45);
	switch(AppState)
		{
		case 0:
			SSD1306_Puts("No Media", &Font_11x18, 1);
			break;
		case 1:
			SSD1306_Puts("Paused", &Font_11x18, 1);
			break;
			SSD1306_Puts("Playing", &Font_11x18, 1);
		case 2:
			break;
		}
	SSD1306_DrawRectangle(rectangle_x, rectangle_y, rectangle_width, rectangle_height, 1);
	SSD1306_DrawFilledRectangle(rectangle_x, rectangle_y, value, rectangle_height, 1);
	SSD1306_UpdateScreen();

	return volume;
}
