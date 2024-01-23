/*
 * CS43L22_I2C.c
 *
 *  Created on: Jan 20, 2024
 *      Author: david
 */

#include "stm32f4xx_hal.h"
#include "CS43L22_I2C.h"

static uint8_t audio_addr_read = 0x95;
static uint8_t audio_addr_write = 0x94;
static I2C_HandleTypeDef *hi2c;
extern I2S_HandleTypeDef hi2s3;

uint8_t estado[2];

void CS43L22_Init(I2C_HandleTypeDef *i2c)
{

	hi2c = i2c;
	//__HAL_UNLOCK(&hi2s3);
	//__HAL_I2S_ENABLE(&hi2s3);
	//POWER CONTROL 1
			  estado[0]=0x02;
			  estado[1]=0x01; //Power down 0x01 / 0x9E para encender
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			  //POWER CONTROL 2
			  estado[0]=0x04;
			  estado[1]=0xAF; //Encendidos auriculares A y B y altavoz apagado
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			  //CLOCKING CONTROL
			  estado[0]=0x05;
			  estado[1]=0x80; //Detección automática de reloj
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			  //INTERFACE CONTROL 1
			  estado[0]=0x06;
			  // Como el bit 5 esta reservaso hay que conocer su estado
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_read, &estado[0], 1, HAL_MAX_DELAY);
			  HAL_I2C_Master_Receive(hi2c, audio_addr_read, &estado[1], 1, HAL_MAX_DELAY);
			  	  estado[1] &= (1 << 5);   // Pone los bit a 0 menos el bit reservado
			  	  estado[1] &= ~(1 << 7);  // Modo esclavo
			  	  estado[1] &= ~(1 << 6);  // Polaridad del reloj no invertida
			  	  estado[1] &= ~(1 << 4);  // Modo DSP OFF
			  	  estado[1] &= ~(1 << 2);  // Left justified, up to 24 bit (default)
			  	  estado[1] |= (1 << 2);
			  	  estado[1] |= (3 << 0);  // Longitud de palabra de 16 bots para I2S
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			  //PASSTHROUGH A
			  estado[0]=0x08;
			  // Como hay bits reservados hay que conocer su estado
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_read, &estado[0], 1, HAL_MAX_DELAY);
			  HAL_I2C_Master_Receive(hi2c, audio_addr_read, &estado[1], 1, HAL_MAX_DELAY);
			  	  estado[1] &= 0xF0;     // Bit [7-4] reservados y el resto a 0
			  	  estado[1] |= (1 << 0); // canal de entrada AIN1A
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);


			  //PASSTHROUGH B
			   estado[0]=0x09;
			   // Como hay bits reservados hay que conocer su estado
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_read, &estado[0], 1, HAL_MAX_DELAY);
			   HAL_I2C_Master_Receive(hi2c, audio_addr_read, &estado[1], 1, HAL_MAX_DELAY);
			   	  estado[1] &= 0xF0;     // Bit [7-4] reservados y el resto a 0
			   	  estado[1] |= (1 << 0); // canal de entrada AIN1B
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			   //MISCELLANEOUS CONTROLS
			   estado[0]=0x0E;
			   estado[1]=0x02; //Para modo de salida I2S//para modo analogico mirar la hoja tecnica
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			   //PLAYBACK_CONTROL 2
			   estado[0]=0x0F;
			   estado[1]=0x00; //Desmutea los auriculares y el altavoz
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			   //ESTABLECER VOLUMEN A 0 DB
			   estado[1]=0x00;
			   estado[0]=0x14;//PASSTHROUHG A VOLUMEN
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			   estado[0]=0x15;//PASSTHROUHG B VOLUMEN
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);


			   estado[0]=0x1A;//PCM A VOLUMEN
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);

			   estado[0]=0x1B;//PCM B VOLUMEN
			   HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);
}

void CS43L22_ON(){
			  estado[0]=0x02;
			  estado[1]=0x9E;
			  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);
}

void CS43L22_OFF(){
	              estado[0]=0x02;
				  estado[1]=0x01;
				  HAL_I2C_Master_Transmit(hi2c, audio_addr_write, estado, 2, HAL_MAX_DELAY);
}
