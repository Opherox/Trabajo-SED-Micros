/*
 * WavPlayer.c
 *
 *  Created on: Jan 30, 2024
 *      Author: Opherox
 */

#include <WavPlayer.h>
#include <CS43L22_I2C.h>

#define PLAYER_BUF_CHANNELS         (2)
#define Buffer_Size 1024
static I2S_HandleTypeDef *PLAYER_hi2s;

static uint16_t wavBuffer[2][Buffer_Size];

static int buffPlaying = 1;
static int buffReading = 0;

static int hasRead = 0;

static bool PLAYER_playing = false;

static FIL PLAYER_songFile;

static int memoryAllocated = 0;

static uint32_t BytesRead = 0;

extern WavHeader header;

static int hasTransfered = 0;

extern char songPlaying[15];

void WavPLAYER_init(I2C_HandleTypeDef *hi2c, I2S_HandleTypeDef *hi2s)
{
  PLAYER_hi2s = hi2s;
  buffReading = 0;
  buffPlaying = 1;
  hasRead = 0;
  hasTransfered = 0;

  CS43L22_Init(hi2c);
}

bool WavPLAYER_isPlaying()
{
  return PLAYER_playing;
}

FRESULT ReadWavHeader(const char* filename, WavHeader* header)
{
    FIL songFile;
    FRESULT result;
    // Open the file
    result = f_open(&songFile, filename, FA_READ);
    if (result != FR_OK)
    {
        //error
        return result;
    }
    // Read the WAV header
    UINT realBytesRead = 0;
    result = f_read(&songFile, header, sizeof(WavHeader), &realBytesRead);
    BytesRead += realBytesRead;
    f_close(&songFile);  // Cerramos el archivo después de leerlo
    // Assuming 16-bit stereo data, adjust accordingly
    return result;
}

/*int WavAllocateMemory()
{
	memoryAllocated = 1;
	for(int i=0; i<2;i++)
	{
		wavBuffer[i] = (uint16_t*)malloc(header.subchunk2Size*header.bitsPerSample);
		if(wavBuffer[i] == NULL)
		{
			for(int j=0; j<2;j++)
			{
				free(wavBuffer[j]);
			}
			memoryAllocated = 0;
			return 0;
		}
	}
	return 1;
}*/

int WavRead(const char filename[], uint16_t buffer[], uint32_t *bytesRead)
{
	if(filename != NULL)
		{
			FRESULT result;
			FIL songFile;
			result = f_open(&songFile, filename, FA_READ); //abrimos el archivo en lectura
			UINT realBytesRead = 0;
			UINT elementsToRead = sizeof(uint16_t) * Buffer_Size;
			if(result != FR_OK)
			{
				printf("Error al abrir el archivo");
				return 1;
			}
			f_lseek(&songFile, *bytesRead); //nos ponemos al principio del archivo + los bytes leidos anteriormente
			if((result = f_read(&songFile, buffer, elementsToRead, &realBytesRead)) == FR_OK) //si la lectura se ha podido realizar
			{
				printf("Lectura de fichero realizada con exito");
				f_close(&songFile);  // Cerramos el archivo después de leerlo
				*bytesRead += realBytesRead; //los bytes leidos son los que ya se habian leido mas los que se ha conseguido leer en esta ocasion
				hasRead = 1;
				return 1;
			}
			else
			{
				printf("Error al leer el archivo");
				f_close(&songFile);  // Cerramos el archivo en caso de error
				return 0;
			}
		}
}

bool WavPlayerPlay()
{
	CS43L22_ON();
	if(songPlaying != NULL)
	{
		   PLAYER_playing = true;
		    if(hasTransfered == 1 && hasRead == 1)
		       {
		       	if(buffReading == 0 && buffPlaying == 1)
		       	{
		       		buffReading = 1;
		       		buffPlaying = 0;
		       	}
		       	else if(buffReading == 1 && buffPlaying == 0)
		       	{
		       		buffReading = 0;
		       		buffPlaying = 1;
		       	}
		       	hasTransfered = 0;
		       	hasRead = 0;
		       }
		    if(hasRead == 0)
		    {
		    	while(!hasRead)
		    	{
		    		WavRead(songPlaying, wavBuffer[buffReading], &BytesRead); //si no ha leido que lea
		    	}
		    	TransmitAudio(wavBuffer[buffPlaying]); //transmitir el otro buffer
		    }
	}
    return true;
}

void WavPlayerStop()
{
    if (PLAYER_playing)
    {
        PLAYER_playing = false;
        CS43L22_OFF();
        f_close(&PLAYER_songFile);
        BytesRead = 0;
    }
}

void WavPLAYER_setVolume(uint16_t vol)
{
	CS43L22_Volume(vol);
}

void TransmitAudio(uint16_t buff[])
{
    HAL_I2S_Transmit_DMA(PLAYER_hi2s, buff, Buffer_Size);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == PLAYER_hi2s)
	{
	hasTransfered = 1;
	}
}
