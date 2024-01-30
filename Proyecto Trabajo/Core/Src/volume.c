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
#include <locale.h>

int AppState;

uint16_t value;
uint16_t volume;
const uint16_t adcval_MAX = 4095;
uint16_t rectangle_x = 5;
uint16_t rectangle_y = 10;
uint16_t rectangle_width = 118;
uint16_t rectangle_height = 10;

char *songPlaying = NULL;
uint32_t fileSize = 0;

void oled_Init()
{
	SSD1306_Init();
}

uint16_t changeVolume(ADC_HandleTypeDef *hadc1, I2C_HandleTypeDef *hi2c1)
{
    /* TODO: eliminar macro cuando se use hi2c1 */
    UNUSED(hi2c1);
    uint16_t adcval;
	//ADC
	HAL_ADC_Start(hadc1);
	if(HAL_ADC_PollForConversion(hadc1, HAL_MAX_DELAY)== HAL_OK){
		adcval = HAL_ADC_GetValue(hadc1);
	}
	HAL_ADC_Stop(hadc1);
	value = (uint16_t)((adcval * rectangle_width)/adcval_MAX);	//From 0 to 118
	volume = (uint16_t)((adcval * 100)/adcval_MAX);				//From 0 to 100
	return volume;
}

void changeAppState(int state)
{
	AppState = state;
}

void changeSongInfo(FILINFO file)
{
	strcpy(songPlaying, file.fname);
	fileSize = file.fsize;
}
void LCDUpdate(void)
{
	SSD1306_Fill(SSD1306_COLOR_BLACK);
	SSD1306_GotoXY(5, 0);
	SSD1306_Puts("Volumen", &Font_7x10, 1);
	SSD1306_DrawRectangle(rectangle_x, rectangle_y, rectangle_width, rectangle_height, 1);
	SSD1306_DrawFilledRectangle(rectangle_x, rectangle_y, value, rectangle_height, 1);
	SSD1306_GotoXY(5,25);
	switch(AppState)
	{
		case 0:
			SSD1306_Puts("No Media", &Font_7x10, 1);
			break;
		case 1:
			SSD1306_Puts("Paused", &Font_7x10, 1);
			break;
		case 2:
			SSD1306_Puts("Playing:", &Font_7x10, 1);
			break;
	}
	SSD1306_GotoXY(5,35);
	if(songPlaying != NULL)
	{
		SSD1306_Puts(songPlaying, &Font_7x10, 1);
	}
	else
	{
		SSD1306_Puts("No song", &Font_7x10, 1);
	}
	SSD1306_GotoXY(5,45);
	if(fileSize != 0)
	{
		char numberString[20];
		float mbSize = fileSize/(1024 * 1024);
		sprintf(numberString, "%.2f Mb", mbSize); //TODO: Revisar xq da numeros gigantes, xq ns si es q no pilla bien el fichero o q
		SSD1306_Puts(numberString, &Font_7x10, 1);
	}
	else
	{

	}

	SSD1306_UpdateScreen();
}
